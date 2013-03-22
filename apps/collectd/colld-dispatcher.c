/*
 * Copyright (c) Feb 2012, Nguyen Quoc Dinh
 * All rights reserved.
 */


/**
 * \file
 *         dispatcher for collect daemon
 * \author
 *         Nguyen Quoc Dinh <nqdinh@hui.edu.vn>
 */


#include <stdlib.h>
#include <string.h>
#include "collectd.h"
#include "colld-dispatcher.h"
#include "collect-view.h"

#define DEBUG NONE
#include "net/uip-debug.h"

#define UDP_IP_BUF   ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

struct{
	u8_t version;
	u8_t id;
	u8_t type;
}collectd_object_header;

/*
 * this function process the request which fall into 2 cases: request, and reply
 */
char collectd_processing(u8_t* const input, const u16_t input_len, collectd_conf_t *collectd_conf) {
	u8_t pos=0;

	u8_t version, id;
	u8_t commandtype;
	u16_t srcport;
	uip_ipaddr_t mnaddr;


	if ((u8_t)input[pos++] != VERSION_01) {
		return FAILURE;
	}
	version = VERSION_01;
	id = (u8_t)input[pos++];

	switch ((u8_t)input[pos++]) {
	case REQUEST:
		/* commandtype (start/stop) -- srcport	*/
		/*the command type and source port of sender (manager) is mentioned in data of the request */
		commandtype = input[pos++];
		if ((commandtype != COMMAND_START) && (commandtype != COMMAND_STOP))
			return FAILURE;
		//srcport sent in big-endian
		srcport = (u16_t)(((u16_t)(input[pos]) << 8) | ((u16_t)(input[pos+1])));
		pos = pos + 2; //end of data for this request
		//sensor receive request from the management host
		uip_ipaddr_copy(&mnaddr, &UDP_IP_BUF->srcipaddr);

		/*save the request to conf*/
		collectd_conf->send_active = commandtype;
		collectd_conf->mnrport = srcport;
		uip_ipaddr_copy(&collectd_conf->mnaddr, &mnaddr);

		/*save header for data response*/
		collectd_object_header.id = id;
		collectd_object_header.version = version;
		collectd_object_header.type = RESPONSE;

		PRINTF("command type = %d\n", commandtype);
		PRINTF("srcport = %d\n", srcport);
		PRINT6ADDR(&mnaddr);
		PRINTF("\n");

		break;

	case RESPONSE:
		//sensor node should not receive a response
		PRINTF("WARNING: a sensor should not receive RESPONSE pkg\n");
		break;
	default:
		return FAILURE;
		break;
	};
	return ERR_NO_ERROR;
}

/*---------------------------------------------------------------------------*/
void collectd_prepare_data(collectd_object_t* collectpayload) {
	static uint8_t seqno;

	/* struct collect_neighbor *n; */
	uint16_t parent_etx;
	uint16_t rtmetric;
	uint16_t num_neighbors;
	uint16_t beacon_interval;
	rpl_parent_t *preferred_parent;
	rimeaddr_t parent;
	rpl_dag_t *dag;


	/*prepare header*/
	collectpayload->version = collectd_object_header.version;
	collectpayload->id = collectd_object_header.id;
	collectpayload->type = collectd_object_header.type;

	seqno++;
	if(seqno == 0) {
		/* Wrap to 128 to identify restarts */
		seqno = 128;
	}
	collectpayload->respdata.seqno = seqno;

	rimeaddr_copy(&parent, &rimeaddr_null);
	parent_etx = 0;

  /* Let's suppose we have only one instance */
  dag = rpl_get_any_dag();
  if(dag != NULL) {
    preferred_parent = dag->preferred_parent;
    if(preferred_parent != NULL) {
      uip_ds6_nbr_t *nbr;
      nbr = uip_ds6_nbr_lookup(&preferred_parent->addr);
      if(nbr != NULL) {
        /* Use parts of the IPv6 address as the parent address, in reversed byte order. */
        parent.u8[RIMEADDR_SIZE - 1] = nbr->ipaddr.u8[sizeof(uip_ipaddr_t) - 2];
        parent.u8[RIMEADDR_SIZE - 2] = nbr->ipaddr.u8[sizeof(uip_ipaddr_t) - 1];
        parent_etx = neighbor_info_get_metric((rimeaddr_t *) &nbr->lladdr) / 2;
      }
    }
    rtmetric = dag->rank;
    beacon_interval = (uint16_t) ((2L << dag->instance->dio_intcurrent) / 1000);
    num_neighbors = RPL_PARENT_COUNT(dag);
  } else {
    rtmetric = 0;
    beacon_interval = 0;
    num_neighbors = 0;
  }

  /* num_neighbors = collect_neighbor_list_num(&tc.neighbor_list); */
  collect_view_construct_message(&collectpayload->respdata.collect_data, &parent,
                                 parent_etx, rtmetric,
                                 num_neighbors, beacon_interval);
}

void collectd_prepare_ouput(collectd_object_t* collectd_object, u8_t* output, u16_t* len){
	u16_t pos =0;
	u8_t i;

	output[pos++] = collectd_object->version;
	output[pos++] = collectd_object->id;
	output[pos++] = collectd_object->type;
	output[pos++] = collectd_object->respdata.seqno;

	COPY_2B_BIGENDIAN(output, pos, collectd_object->respdata.collect_data.len); pos+=2;
	COPY_2B_BIGENDIAN(output, pos, collectd_object->respdata.collect_data.clock); pos+=2;
	COPY_2B_BIGENDIAN(output, pos, collectd_object->respdata.collect_data.timesynch_time); pos+=2;
	COPY_2B_BIGENDIAN(output, pos, collectd_object->respdata.collect_data.cpu); pos+=2;
	COPY_2B_BIGENDIAN(output, pos, collectd_object->respdata.collect_data.lpm); pos+=2;
	COPY_2B_BIGENDIAN(output, pos, collectd_object->respdata.collect_data.transmit); pos+=2;
	COPY_2B_BIGENDIAN(output, pos, collectd_object->respdata.collect_data.listen); pos+=2;
	COPY_2B_BIGENDIAN(output, pos, collectd_object->respdata.collect_data.parent); pos+=2;
	COPY_2B_BIGENDIAN(output, pos, collectd_object->respdata.collect_data.parent_etx); pos+=2;
	COPY_2B_BIGENDIAN(output, pos, collectd_object->respdata.collect_data.current_rtmetric); pos+=2;
	COPY_2B_BIGENDIAN(output, pos, collectd_object->respdata.collect_data.num_neighbors); pos+=2;
	COPY_2B_BIGENDIAN(output, pos, collectd_object->respdata.collect_data.beacon_interval); pos+=2;

	PRINTF("cpu=%d, lpm=%d, trans=%d, list=%d\n",
			collectd_object->respdata.collect_data.cpu,
			collectd_object->respdata.collect_data.lpm,
			collectd_object->respdata.collect_data.transmit,
			collectd_object->respdata.collect_data.listen);

	for (i=0; i < MAX_NUM_SENSOR; i++){
		COPY_2B_BIGENDIAN(output, pos, collectd_object->respdata.collect_data.sensors[i]); pos+=2;
	}
	*len = pos;
}
char collectd_common_send(struct uip_udp_conn* client_conn,collectd_conf_t* collectd_conf){
	collectd_object_t collectd_object;
	u8_t output[MAX_BUF_SIZE];
	u16_t len;

	collectd_prepare_data(&collectd_object);

	if (sizeof(collectd_object) > MAX_BUF_SIZE)
		return ERR_MEMORY_ALLOCATION;

	collectd_prepare_ouput(&collectd_object, output, &len);
	uip_udp_packet_sendto(client_conn, output,
			len, &collectd_conf->mnaddr, UIP_HTONS(collectd_conf->mnrport));
	return ERR_NO_ERROR;
}
