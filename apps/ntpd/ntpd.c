/*
 * NTP client implementation for Contiki
 *
 * NTPv4 - RFC 5905
 *
 * Copyright (c) 2011, 2012 Josef Lusticky
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *		notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *		notice, this list of conditions and the following disclaimer in the
 *		documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *		may be used to endorse or promote products derived from this software
 *		without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.	IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*
 * Nguyen Quoc Dinh nqdinh@hui.edu.vn
 * Mar 2013
 */

#include <stdio.h>
#include <string.h>
#include "ntpd.h"

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#define POLL_INTERVAL (1 << TAU)
#define SEND_INTERVAL POLL_INTERVAL * CLOCK_SECOND

#define UDP_EXAMPLE_ID  190
static struct uip_udp_conn *client_conn;
static struct ntp_msg msg;
static uip_ipaddr_t server_ipaddr;

/*---------------------------------------------------------------------------*/
PROCESS(ntpd_process, "ntpd process");

/*---------------------------------------------------------------------------*/
static void
set_global_address(void)
{

/* The choice of server address determines its 6LoPAN header compression.
 * (Our address will be compressed Mode 3 since it is derived from our link-local address)
 * Obviously the choice made here must also be selected in udp-server.c.
 *
 * For correct Wireshark decoding using a sniffer, add the /64 prefix to the 6LowPAN protocol preferences,
 * e.g. set Context 0 to aaaa::.  At present Wireshark copies Context/128 and then overwrites it.
 * (Setting Context 0 to aaaa::1111:2222:3333:4444 will report a 16 bit compressed address of aaaa::1111:22ff:fe33:xxxx)
 *
 * Note the IPCMV6 checksum verification depends on the correct uncompressed addresses.
 */

#if 1
/* Mode 1 - 64 bits inline */
   uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);
#elif 0
   uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0x212, 0x7401, 1, 0x101);
#elif 0
/* Mode 2 - 16 bits inline */
  uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);
#else
/* Mode 3 - derived from server link-local (MAC) address */
  uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0x0250, 0xc2ff, 0xfea8, 0xcd1a); //redbee-econotag
#endif
}
/*---------------------------------------------------------------------------*/
#ifndef BROADCAST_MODE            // this function sends NTP client message to REMOTE_HOST
static void timeout_handler(void) {
	ntp_client_send(client_conn, server_ipaddr, msg);
}
#endif
/*------------------------------------------------------------------*/
static void tcpip_handler(void) {
	PRINTF ("Recv a packet \n");
	ntp_adjust_time();
}

PROCESS_THREAD(ntpd_process, ev, data)
{
	static struct etimer et;
	static struct etimer et_check_c;

	PROCESS_BEGIN();

	PROCESS_PAUSE();

	set_global_address();

	PRINTF("UDP client process started\n");

	client_conn = udp_new(NULL, UIP_HTONS(SERVER_PORT), NULL);        // remote server port

	udp_bind(client_conn, UIP_HTONS(CLIENT_PORT));     // local client port

	msg.ppoll = TAU;              // log2(poll_interval)

	// Send interval in clock ticks
	clock_set(500,500);

	PRINTF("Created a connection with the server ");
	PRINT6ADDR(&client_conn->ripaddr);
	PRINTF(" local/remote port %u/%u\n",
	UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));

	etimer_set(&et, 6 * CLOCK_SECOND);
	PRINTF ("WAIT 6 second \n");
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

	etimer_set(&et_check_c, 10 * CLOCK_SECOND);
#ifndef BROADCAST_MODE
	// ask for time
	msg.refid = UIP_HTONL(0x494e4954);    // INIT string in ASCII
	timeout_handler();
	msg.refid = 0;

	etimer_set(&et, SEND_INTERVAL);       // wait SEND_INTERVAL before sending next request

	PRINTF("WAIT SEND INTERVAL: %d\n",POLL_INTERVAL);
#endif

	while(1){
		PROCESS_WAIT_EVENT();

		if(ev == tcpip_event) {
			tcpip_handler();
		}
#ifndef BROADCAST_MODE
		if(etimer_expired(&et)) {
			timeout_handler();
			etimer_reset(&et);      // wait again SEND_INTERVAL seconds
		}
//else if(ev == PROCESS_EVENT_MSG)  // another application wants us to synchronise
//    {
//      timeout_handler();
//    }
#endif
		if (etimer_expired(&et_check_c)) {
			unsigned long check_second;
			rtimer_clock_t check_clock_counter;
			check_second=clock_seconds();
			check_clock_counter=clock_counter();
			PRINTF ("current time: %lu%lu,%lu%lu \n",
					check_second/65536, check_second%65536,
					check_clock_counter/65536, check_clock_counter%65536);
			etimer_reset(&et_check_c);
		}
	}
	PROCESS_END();
}
