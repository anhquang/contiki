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
	r = jsmn_parse(&p, (char*)input, tokens, MAX_TOKEN);
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

/*
 * this function produce a string @buf in form of json data
 * example of buf:
 *
 * {'clk':%d,'syn':%d,'cpu':%d,'lpm':%d,'tras':%d,'lst':%d,
 * 'parent':%s,'etx':%d,'rt':%d,\'nbr':%d,'bea_itv':%d,'sen':%d}
 *
 */
void collectd_prepare_data()
{
	uint16_t parent_etx;
	uint16_t rtmetric;
	uint16_t num_neighbors;
	uint16_t beacon_interval;
	rpl_parent_t *preferred_parent;
	uip_lladdr_t lladdr_parent;
	rpl_dag_t *dag;

//copied from collect-view.c
	static unsigned long last_cpu, last_lpm, last_transmit, last_listen;
	unsigned long cpu, lpm, transmit, listen;
	u16_t clock, timesynch_time;

	clock = clock_time();
#if TIMESYNCH_CONF_ENABLED
	timesynch_time = timesynch_time();
#else /* TIMESYNCH_CONF_ENABLED */
	timesynch_time = 0;
#endif /* TIMESYNCH_CONF_ENABLED */

	/*save to buf */
	blen = 0;
	ADD("{'clk':%u,'syn':%u,", clock, timesynch_time);

	energest_flush();
	cpu = energest_type_time(ENERGEST_TYPE_CPU) - last_cpu;
	lpm = energest_type_time(ENERGEST_TYPE_LPM) - last_lpm;
	transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT) - last_transmit;
	listen = energest_type_time(ENERGEST_TYPE_LISTEN) - last_listen;

	/* Make sure that the values are within 16 bits. If they are larger,
	 we scale them down to fit into 16 bits. */
	//TODO: why do i need to scale down to 16 bit?
	while(cpu >= 65536ul || lpm >= 65536ul ||
			transmit >= 65536ul || listen >= 65536ul) {
		cpu /= 2;
		lpm /= 2;
		transmit /= 2;
		listen /= 2;
	}
	/* prepare for next calling */
	last_cpu = energest_type_time(ENERGEST_TYPE_CPU);
	last_lpm = energest_type_time(ENERGEST_TYPE_LPM);
	last_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
	last_listen = energest_type_time(ENERGEST_TYPE_LISTEN);

	/* save to buf */
	ADD("'cpu':%u,'lpm':%u,'tras':%u,'lst':%u,",
			(u16_t)cpu, (u16_t)lpm,
			(u16_t)transmit, (u16_t)listen);

	/* initial value, if there's not any dag */
	parent_etx = 0;
	rtmetric = 0;
	beacon_interval = 0;
	num_neighbors = 0;
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
	}

	char lladdr_parent_str[30];
	u8_t lladdr_str_len;
	lladdr_str_len = lladdr_print(&lladdr_parent, lladdr_parent_str, 30);

	//ADD_N(lladdr_str, lladdr_str_len);
	ADD("'parent':%s,", lladdr_parent_str);

	ADD("'etx':%u,'rt':%u,'nbr':%u,'bea_itv':%u,",
			parent_etx, rtmetric, num_neighbors,
			beacon_interval);

	//collectd_arch_read_sensors();
	ADD("}");
}

/*
 * convert lladdr to string
 */
u8_t lladdr_print(const uip_lladdr_t *addr, char *lladdr_str, u8_t strlen)
{
	u8_t i;
	u8_t len=0;
	for(i = 0; i < sizeof(uip_lladdr_t); i++) {
		if(i > 0) {
			len += snprintf(&lladdr_str[len], strlen - len, ":");
		}
		len += snprintf(&lladdr_str[len], strlen - len, "%02x", addr->addr[i]);
	}
	//make end of string character
	lladdr_str[len] = 0;
	return len;
}

char collectd_common_send(struct uip_udp_conn* client_conn,collectd_conf_t* collectd_conf){
	collectd_prepare_data();
	PRINTF("%s\n", buf);

	uip_udp_packet_sendto(client_conn, buf,
			blen, &collectd_conf->mnaddr, UIP_HTONS(collectd_conf->mnrport));

	return COLLECTD_ERROR_NO_ERROR;
}
