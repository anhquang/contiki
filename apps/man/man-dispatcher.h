/*
 * Copyright (c) Feb 2012, Nguyen Quoc Dinh
 * All rights reserved.
 */


/**
 * \file
 *         dispatcher for collection daemon
 * \author
 *         Nguyen Quoc Dinh <nqdinh@hui.edu.vn>
 */

#ifndef __COLLD_DISPATCHER_PROTOCOL_H__
#define __COLLD_DISPATCHER_PROTOCOL_H__

/*includes*/
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-conf.h"
#include "man.h"

char man_processing(u8_t* const input, const u16_t input_len, man_conf_t *man_conf);
void man_prepare_data(void);
char man_common_send(struct uip_udp_conn* client_conn,man_conf_t* man_conf);
u8_t lladdr_print(const uip_lladdr_t *addr, char *lladdr_str, u8_t strlen);
#endif
