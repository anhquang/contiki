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
#include <errno.h>
#include <limits.h>

#include "contiki.h"
#include "net/uiplib.h"
#include "collectd.h"
#include "colld-dispatcher.h"
#include "jsmn.h"
#include "jsmn-utility.h"

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#define MAX_TOKEN 	20
#define TOKEN_LEN	20

#define UDP_IP_BUF   ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

/* one reply at a time*/
static char buf[MAX_BUF_SIZE];
static int blen;
#define ADD(...) do {                                                   \
    blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__);      \
  } while(0)

/*
 * this function process the request which fall into 2 cases: request, and reply
 * an example of receiving request:
  		{
		'status': 'start',		//start/stop
		'update': 1,			//in second
		'addr': 'aaaa::1',
		'port': 20000
		}
 */
char collectd_processing(u8_t* const input, const u16_t input_len, collectd_conf_t *collectd_conf) {
	static jsmn_parser p;
	static jsmntok_t tokens[MAX_TOKEN];
	static char value[TOKEN_LEN];

	//values to save temp from input json (before make sure json msg is valid)
	u8_t commandtype;
	u16_t update;
	u16_t srcport;
	uip_ipaddr_t mnaddr;
	int r;

	input[input_len] = 0;
	PRINTF("%s\n", input);
	jsmn_init(&p);
	r = jsmn_parse(&p, input, tokens, MAX_TOKEN);
	check(r == JSMN_SUCCESS);

	//get status
	check(js_get(input, tokens, MAX_TOKEN, "status", value, MAX_TOKEN) == JSMN_TOKEN_SUCCESS);
	check( (strcmp(value, "start")==0) || (strcmp(value, "stop")==0) );
	commandtype = (strcmp(value, "start") == 0)? COMMAND_START : COMMAND_STOP;
	//PRINTF("start = %d\n", commandtype);

	check(js_get(input, tokens, MAX_TOKEN, "update", value, MAX_TOKEN) == JSMN_TOKEN_SUCCESS);
	errno = 0;
	update = (u16_t)strtol(value, NULL, 10);		//convert to base 10
	check(!(errno == ERANGE || (errno != 0 && update == 0)));
	//PRINTF("update = %d\n", update);

	check(js_get(input, tokens, MAX_TOKEN, "addr", value, MAX_TOKEN) == JSMN_TOKEN_SUCCESS);
	check(uiplib_ipaddrconv(value, &mnaddr) == 1);
	//PRINT6ADDR(&mnaddr);
	//PRINTF("\n");

	check(js_get(input, tokens, MAX_TOKEN, "port", value, MAX_TOKEN) == JSMN_TOKEN_SUCCESS);
	errno = 0;
	srcport = (u16_t)strtol(value, NULL, 10);		//convert to base 10
	check(!(errno == ERANGE || (errno != 0 && update == 0)));
	//PRINTF("port = %d\n", srcport);

	/*save the request to conf*/
	collectd_conf->send_active = commandtype;
	collectd_conf->update_freq_in_sec = update;
	collectd_conf->mnrport = srcport;
	uip_ipaddr_copy(&collectd_conf->mnaddr, &mnaddr);

	return COLLECTD_ERROR_NO_ERROR;
}

/*---------------------------------------------------------------------------*/
void collectd_prepare_data() {
	//u8_t * seqno = "{'clk':%d,'syn':%d,'cpu':%d,'lpm':%d,'tras':%d,'lst':%d,'parent':%s,'etx':%d,'rt':%d,\
	//		'nbr':%d,'bea_itv':%d,'sen':%d}";

	uint16_t parent_etx;
	uint16_t rtmetric;
	uint16_t num_neighbors;
	uint16_t beacon_interval;
	rpl_parent_t *preferred_parent;
	uip_lladdr_t lladdr_parent;
	rimeaddr_t parent;
	rpl_dag_t *dag;

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
				//PRINT6ADDR(&nbr->lladdr);
				memcpy(&lladdr_parent, &nbr->lladdr, sizeof(uip_lladdr_t));
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
	blen = 0;
	ADD("{'rt':%d,'bea_itvn':%d}",
			rtmetric, beacon_interval);

}

char collectd_common_send(struct uip_udp_conn* client_conn,collectd_conf_t* collectd_conf){
	collectd_prepare_data();
	PRINTF("%s", buf);

//	uip_udp_packet_sendto(client_conn, output,
//			len, &collectd_conf->mnaddr, UIP_HTONS(collectd_conf->mnrport));

	return COLLECTD_ERROR_NO_ERROR;
}
