/*
 * Copyright (c) Feb 2012, Nguyen Quoc Dinh
 * All rights reserved.
 */


/**
 * \file
 *         dispatcher for network tree daemon
 * \author
 *         Nguyen Quoc Dinh <nqdinh@hui.edu.vn>
 */


#include <stdlib.h>
#include <string.h>
#include "networktreed.h"
#include "nwktr-dispatcher.h"

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#define UDP_IP_BUF   ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

/* this function process the request which fall into 3 cases: request, forcerequest, and reply
 *
 */
char nwkgraph_processing(u8_t* const input, const u16_t input_len, struct uip_udp_conn* const udpconn) {
	u8_t pos;
	u16_t srcport;
	u8_t responselen;
	networktree_object_t nwktree;
	u8_t response[MAX_BUF_SIZE];

	uip_ipaddr_t mnaddr;
	u8_t i;

	pos = 0;

	if ((u8_t)input[pos++] != VERSION_01) {
		return FAILURE;
	}
	nwktree.version = VERSION_01;
	nwktree.id = (u8_t)input[pos++];

	switch ((u8_t)input[pos++]) {
	case REQUEST:
		//the source port of sender (manager) is mentioned in data of the request
		srcport = (u16_t)(((u16_t)(input[pos]) << 8) | ((u16_t)(input[pos+1])));
		pos = pos + 2; //end of data for this request
		//sensor receive request from the management host
		uip_ipaddr_copy(&mnaddr, &UDP_IP_BUF->srcipaddr);

		nwktree.type = RESPONSE;

		/* prepare RESPONSE data */
		uip_ipaddr_copy((uip_ipaddr_t*)&nwktree.varbind_object, uip_ds6_defrt_choose()); //then call nwkgraph_prepare_respacket_data_addr
		/* prepare RESPONSE buffer */
		if (nwkgraph_prepare_respacket_header(&nwktree, response, &responselen, MAX_BUF_SIZE) != ERR_NO_ERROR)
			return FAILURE;
		if (nwkgraph_prepare_respacket_data_addr(&nwktree, response, &responselen, MAX_BUF_SIZE) != ERR_NO_ERROR)
			return FAILURE;

		PRINTF("A message reply to ");
		PRINT6ADDR(&UDP_IP_BUF->srcipaddr);
		PRINTF("\n");
		//send reply to management host
		uip_udp_packet_sendto(udpconn, response, responselen, &mnaddr, srcport);

		/* send force-request to my parent
		 * data for this protocol contain
		 * (1) manager remote port (which mentioned in request data
		 * (2) manager address
		 */
		nwktree.type = FORCEREQUEST;
		/* prepare FORCEREQUEST data */
		nwktree.varbind_object.forcereq_data.mnrport = srcport;
		uip_ipaddr_copy((uip_ipaddr_t*)&nwktree.varbind_object.forcereq_data.mnaddr, &mnaddr);
		/* prepare FORCEREQUEST buffer */
		if (nwkgraph_prepare_respacket_header(&nwktree, response, &responselen, MAX_BUF_SIZE) != ERR_NO_ERROR)
			return FAILURE;
		if (nwkgraph_prepare_respacket_data_forcereq(&nwktree, response, &responselen, MAX_BUF_SIZE) != ERR_NO_ERROR)
			return FAILURE;

		PRINTF("A request message to ");
		PRINT6ADDR(uip_ds6_defrt_choose());
		PRINTF("\n");
		//send reply to management host
		uip_udp_packet_sendto(udpconn, response, responselen, uip_ds6_defrt_choose(), UIP_HTONS(LISTEN_PORT));

		break;

	case FORCEREQUEST:

		srcport = (u16_t)(((u16_t)(input[pos]) << 8) | ((u16_t)(input[pos+1])));
		pos = pos + 2;
		for (i=0; i<16; i++)
			mnaddr.u8[i] = input[pos++];

		nwktree.type = RESPONSE;

		/* prepare RESPONSE data */
		uip_ipaddr_copy((uip_ipaddr_t*)&nwktree.varbind_object, uip_ds6_defrt_choose()); //then call nwkgraph_prepare_respacket_data_addr
		/* prepare RESPONSE buffer */
		if (nwkgraph_prepare_respacket_header(&nwktree, response, &responselen, MAX_BUF_SIZE) != ERR_NO_ERROR)
			return FAILURE;
		if (nwkgraph_prepare_respacket_data_addr(&nwktree, response, &responselen, MAX_BUF_SIZE) != ERR_NO_ERROR)
			return FAILURE;

		PRINTF("Receive forcerequest from ");
		PRINT6ADDR(&UDP_IP_BUF->srcipaddr);
		PRINTF(", then reply to man at ");
		PRINT6ADDR(&mnaddr);
		PRINTF("\n");

		//send reply to management host
		uip_udp_packet_sendto(udpconn, response, responselen, &mnaddr, srcport);

		break;
	case RESPONSE:
		break;
	default:
		return FAILURE;
		break;
	};
	return ERR_NO_ERROR;
}

char nwkgraph_prepare_respacket_header(networktree_object_t *nwktree, u8_t* response, u8_t *responselen, u8_t maxreslen){
	u8_t pos;
	pos = 0;
	response[pos++] = nwktree->version;
	response[pos++] = nwktree->id;
	response[pos++] = nwktree->type;
	*responselen = pos;
	//TODO: how to check maxreslen before assigm value to response?
	if (pos > maxreslen)
		return ERR_MEMORY_ALLOCATION;
	else
		return ERR_NO_ERROR;
}

char nwkgraph_prepare_respacket_data_addr(networktree_object_t *nwktree, u8_t* response, u8_t *responselen, u8_t maxreslen){
	u8_t pos;
	u8_t i;
	pos = *responselen;
	for (i=0; i<16; i++)
		response[pos++] = nwktree->varbind_object.parentaddr.u8[i];
	*responselen = pos;
	//TODO: how to check maxreslen before assigm value to response?
	if (pos > maxreslen)
		return ERR_MEMORY_ALLOCATION;
	else
		return ERR_NO_ERROR;
}

char nwkgraph_prepare_respacket_data_forcereq(networktree_object_t *nwktree, u8_t* response, u8_t *responselen, u8_t maxreslen){
	u8_t pos;
	u8_t i;
	u16_t mnrport;

	mnrport = nwktree->varbind_object.forcereq_data.mnrport;
	pos = *responselen;
	//copy port (big endian manner
	response[pos++] = (u8_t)(mnrport >> 8);
	response[pos++] = (u8_t)(mnrport % 256);
	for (i=0; i<16; i++)
		response[pos++] = nwktree->varbind_object.forcereq_data.mnaddr.u8[i];
	*responselen = pos;
	//TODO: how to check maxreslen before assign value to response?
	if (pos > maxreslen)
		return ERR_MEMORY_ALLOCATION;
	else
		return ERR_NO_ERROR;
}
char nwkgraph_prepare_respacket_data_port(networktree_object_t *nwktree, u8_t* response, u8_t *responselen, u8_t maxreslen){
	return ERR_NO_ERROR;
}
