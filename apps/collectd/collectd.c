/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Collect network tree from leave to gateway generated by routing algo (i.e ROLL for current Contiki NWK layer)
 * \modifier
 *         Nguyen Quoc Dinh <nqdinh@hui.edu.vn>
 */

#include <stdio.h>
#include <string.h>
#include "collectd.h"
#include "colld-dispatcher.h"

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

static struct uip_udp_conn *client_conn;
static collectd_conf_t collectd_conf;

/*---------------------------------------------------------------------------*/
PROCESS(collectd_client_process, "Collectd client process");
PROCESS(collectd_sending_process, "Collectd sending process");
//AUTOSTART_PROCESSES(&collect_common_process);
//AUTOSTART_PROCESSES(&collect_common_process);

/*---------------------------------------------------------------------------*/
static void collectd_udp_handler(void) {
	  if(uip_newdata()) {
		  char rst;
		  rst = collectd_processing((uint8_t*)uip_appdata, uip_datalen(), &collectd_conf);
		  PRINTF("Collectd udp handler return with %d\n", rst);
	  }
}
/*---------------------------------------------------------------------------*/
void collectd_conf_init(collectd_conf_t *conf){
	conf->send_active = SEND_ACTIVE_NO;
	conf->update_freq_in_sec = DEFAULT_UPDATE_PERIOD;	//this value may update via snmp
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(collectd_client_process, ev, data)
{
	PROCESS_BEGIN();

	collectd_conf_init(&collectd_conf);

	/* new connection with remote host */
	client_conn = udp_new(NULL, UIP_HTONS(0), NULL);
	udp_bind(client_conn, UIP_HTONS(COLLECTD_CLIENT_PORT));

	PRINTF("Created a connection with the server ");

	while(1) {
		PROCESS_YIELD();
		if(ev == tcpip_event) {
			collectd_udp_handler();
		}
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(collectd_sending_process, ev, data)
{
	static struct etimer period_timer, wait_timer;
	PROCESS_BEGIN();
	/* Send a packet every 60-62 seconds. */
	etimer_set(&period_timer, CLOCK_SECOND * DEFAULT_UPDATE_PERIOD);

	while(1) {
		PROCESS_WAIT_EVENT();
		if(ev == PROCESS_EVENT_TIMER) {
			if (data == &period_timer) {
				etimer_reset(&period_timer);		//TODO: reset the period from collectd_conf.update freq
				if (collectd_conf.send_active == SEND_ACTIVE_YES)
					etimer_set(&wait_timer, random_rand() % (CLOCK_SECOND * RANDWAIT));

			} else if(data == &wait_timer) {
				/* Time to send the data */
				PRINTF("Time to send the data\n");
				collectd_common_send(client_conn, &collectd_conf);
			}
		}
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/