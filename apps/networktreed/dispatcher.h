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

#include "contiki-net.h"
#include "networktreed.h"

s8_t dispatch(u8_t* const input, const u16_t input_len, u8_t* output, u16_t* output_len, const u16_t max_output_len, struct uip_udp_conn *replyconn);


#endif
