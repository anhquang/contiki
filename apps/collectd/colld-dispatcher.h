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
#include "collectd.h"

char collectd_processing(u8_t* const input, const u16_t input_len, collectd_conf_t *collectd_conf);
void collectd_prepare_data(void);
char collectd_common_send(struct uip_udp_conn* client_conn,collectd_conf_t* collectd_conf);
#endif
