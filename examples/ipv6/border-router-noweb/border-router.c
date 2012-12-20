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
 *         border-router
 * \author
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Nicolas Tsiftes <nvt@sics.se>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/rpl/rpl.h"

#include "net/netstack.h"
#include "dev/button-sensor.h"
#include "dev/slip.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

uint16_t dag_id[] = {0x1111, 0x1100, 0, 0, 0, 0, 0, 0x0011};

extern uip_ds6_nbr_t uip_ds6_nbr_cache[];
extern uip_ds6_route_t uip_ds6_routing_table[];

static uip_ipaddr_t prefix;
static uint8_t prefix_set;

static struct uip_udp_conn *server_conn;
#define SA_SER_PORT 3001
#define SNMPD_PORT	3003
#define MAX_PAYLOAD_LEN 128

//#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
//#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[UIP_LLIPH_LEN])
#define UIP_UDPIP_BUF   ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

#define BUF_USES_STACK 0
#if BUF_USES_STACK
	static char *bufptr, *bufend;
	#define ADD(...) do {                                                   \
		bufptr += snprintf(bufptr, bufend - bufptr, __VA_ARGS__);      \
	} while(0)
#else
	static char buf[556];
	static int blen;
	#define ADD(...) do {                                                   \
		blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__);      \
	} while(0)
#endif

PROCESS(border_router_process, "Border router process");
PROCESS(snmp_assistance_process, "SNMP assistance process");

/* No webserver */
AUTOSTART_PROCESSES(&border_router_process, &snmp_assistance_process);

/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTA("Server IPv6 addresses:\n");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINTA(" ");
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTA("\n");
    }
  }
}
/*---------------------------------------------------------------------------*/
void
request_prefix(void)
{
  /* mess up uip_buf with a dirty request... */
  uip_buf[0] = '?';
  uip_buf[1] = 'P';
  uip_len = 2;
  slip_send();
  uip_len = 0;
}
/*---------------------------------------------------------------------------*/
void
set_prefix_64(uip_ipaddr_t *prefix_64)
{
  uip_ipaddr_t ipaddr;
  memcpy(&prefix, prefix_64, 16);
  memcpy(&ipaddr, prefix_64, 16);
  prefix_set = 1;
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(border_router_process, ev, data)
{
  static struct etimer et;
  rpl_dag_t *dag;

  PROCESS_BEGIN();

/* While waiting for the prefix to be sent through the SLIP connection, the future
 * border router can join an existing DAG as a parent or child, or acquire a default 
 * router that will later take precedence over the SLIP fallback interface.
 * Prevent that by turning the radio off until we are initialized as a DAG root.
 */
  prefix_set = 0;
  NETSTACK_MAC.off(0);

  PROCESS_PAUSE();

  //SENSORS_ACTIVATE(button_sensor);

  PRINTF("RPL-Border router started\n");
#if 0
   /* The border router runs with a 100% duty cycle in order to ensure high
     packet reception rates.
     Note if the MAC RDC is not turned off now, aggressive power management of the
     cpu will interfere with establishing the SLIP connection */
  NETSTACK_MAC.off(1);
#endif
 
  /* Request prefix until it has been received */
  while(!prefix_set) {
    etimer_set(&et, CLOCK_SECOND);
    request_prefix();
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  dag = rpl_set_root(RPL_DEFAULT_INSTANCE,(uip_ip6addr_t *)dag_id);
  if(dag != NULL) {
    rpl_set_prefix(dag, &prefix, 64);
    PRINTF("created a new RPL dag\n");
  }

  /* Now turn the radio on, but disable radio duty cycling.
   * Since we are the DAG root, reception delays would constrain mesh throughput.
   */
  NETSTACK_MAC.off(1);
  
#if DEBUG || 1
  print_local_addresses();
#endif

  while(1) {
    PROCESS_YIELD();
    if (ev == sensors_event && data == &button_sensor) {
    	//the global dag repair disable temporary
    	PRINTF("Initiating global repair (but disable)\n");
    	rpl_repair_root(RPL_DEFAULT_INSTANCE);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(snmp_assistance_process, ev, data) {
	PROCESS_BEGIN();
	//open a UDP server socket
	server_conn = udp_new(NULL, NULL, NULL);				//accepting connection without client limitation
	udp_bind(server_conn, UIP_HTONS(SA_SER_PORT));			//listen on this port

	while(1) {
		PROCESS_YIELD();
		if(ev == tcpip_event) {
			PRINTF("goto udp server of snmp_assistance_process\n");
			snmp_ass_handler();
		}
	}
	//accept request
	//return list of neighbor (motes in network)
	PROCESS_END();
}

void snmp_ass_handler(void) {

  //char buf[MAX_PAYLOAD_LEN];

  if(uip_newdata()) {

	((char *)uip_appdata)[uip_datalen()] = 0;		//end character
	PRINTF("Server received: '%s' from ", (char *)uip_appdata);
	PRINT6ADDR(&UIP_UDPIP_BUF->srcipaddr);
	PRINTF("\n");

	blen = 0;
	yield_routing_info();
	PRINTF("Responding with message: ");
	PRINTF("%s\n", buf);
	PRINTF("Len = %u and %u", blen, strlen(buf));

    /*create new udp connection, with destination {ipaddress, port} set to the connector */
	/*
    static struct uip_udp_conn *new_conn;
    uint16_t conn_udp_port;
    conn_udp_port = UIP_HTONS((uint16_t)UIP_UDPIP_BUF->srcport);
    new_conn = udp_new(NULL, UIP_HTONS(conn_udp_port), NULL);			//from the source
    udp_bind(new_conn, UIP_HTONS(SA_SER_PORT));							//I listen on this port
    uip_ipaddr_copy(&new_conn->ripaddr, &UIP_UDPIP_BUF->srcipaddr);
    */

	/*this udp server will send back the list of neighbor and router built through rpl*/

	uip_ipaddr_copy(&server_conn->ripaddr, &UIP_UDPIP_BUF->srcipaddr);
	server_conn->rport = UIP_UDPIP_BUF->srcport;
	uip_udp_packet_send(server_conn, buf, strlen(buf));

    /* Restore server connection to allow data from any node */
    memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
    server_conn->rport = 0;
  }
}

/*---------------------------------------------------------------------------*/
static void
ipaddr_add(const uip_ipaddr_t *addr)
{
  uint16_t a;
  int i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) ADD("::");
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        ADD(":");
      }
      ADD("%x", a);
    }
  }
}
/*---------------------------------------------------------------------------*/
void yield_routing_info() {

	//TODO: hey, this long answer back require a fragment in reply (by breaking into sequence of line)
#ifdef LONG_SNMP_ASS
	int i;
	ADD("{");
	ADD("'Neighbor':(");
	for (i=0; i<UIP_DS6_NBR_NB; i++){
		if(uip_ds6_nbr_cache[i].isused) {
			ADD("'");
			ipaddr_add(&uip_ds6_nbr_cache[i].ipaddr);
			ADD("',");
		}
	}
	ADD("),");

	//router
	ADD("'Router':[");
	for (i=0; i<UIP_DS6_ROUTE_NB; i++){
		if(uip_ds6_routing_table[i].isused) {
			ADD("{");
			ADD("'ip':'");
			ipaddr_add(&uip_ds6_routing_table[i].ipaddr);
			ADD("',");
			//ADD("'length'", &uip_ds6_routing_table[i].length);
			ADD("'via':'");
			ipaddr_add(&uip_ds6_routing_table[i].nexthop);
			ADD("',");
			ADD("'lifetime': '%u'", uip_ds6_routing_table[i].state.lifetime);
			ADD("}");
			ADD(",");
		}
	}
	ADD("]");
	ADD("}");
#else
	//TODO: this is not a robust way to sent reply, a fragment (by sending line in sequence) would be better

	uip_ds6_route_t *r;
	ADD("{'Router':[");
	for(r = uip_ds6_route_list_head(); r != NULL; r = list_item_next(r)) {
		ADD("'");
		ipaddr_add(&r->ipaddr);
		ADD("',");
	}
	ADD("]}");
#endif
}
