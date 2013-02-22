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

#ifndef __NWKTREE_PROTOCOL_H__
#define __NWKTREE_PROTOCOL_H__

/*includes*/
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-conf.h"
#include "networktreed.h"

char nwkgraph_processing(u8_t* const input, const u16_t input_len, struct uip_udp_conn *udpconn);
char nwkgraph_prepare_respacket_header(networktree_object_t* const nwktree, u8_t* response, u8_t *responselen, u8_t maxreslen);
char nwkgraph_prepare_respacket_data_addr(networktree_object_t* const nwktree, u8_t* response, u8_t *responselen, u8_t maxreslen);
char nwkgraph_prepare_respacket_data_forcereq(networktree_object_t* const nwktree, u8_t* response, u8_t *responselen, u8_t maxreslen);
char nwkgraph_prepare_respacket_data_port(networktree_object_t* const nwktree, u8_t* response, u8_t *responselen, u8_t maxreslen);

#endif
