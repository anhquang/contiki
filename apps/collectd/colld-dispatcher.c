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

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#define UDP_IP_BUF   ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

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

void collectd_common_send(struct uip_udp_conn* client_conn,collectd_conf_t* collectd_conf){
	collectd_object_t collectd_object;

	collectd_prepare_data(&collectd_object);
	uip_udp_packet_sendto(client_conn, &collectd_object,
			sizeof(collectd_object), &collectd_conf->mnaddr, UIP_HTONS(collectd_conf->mnrport));
}
