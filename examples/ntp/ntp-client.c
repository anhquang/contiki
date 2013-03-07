#include "ntpd.h"


#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-udp-packet.h"
#include "sys/ctimer.h"

#include <stdio.h>
#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#define POLL_INTERVAL (1 << TAU)
#define SEND_INTERVAL POLL_INTERVAL * CLOCK_SECOND

#define UDP_EXAMPLE_ID  190
static struct uip_udp_conn *client_conn;
static struct ntp_msg msg;
static uip_ipaddr_t server_ipaddr;
/*----------------------------------------------------------------------------*/
PROCESS(ntpd_process, "ntpd");
AUTOSTART_PROCESSES(&ntpd_process);
/*----------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Client IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
	uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;

  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

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

#if 0  
/* Mode 1 - 64 bits inline */
   uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);
#elif 1
/* Mode 2 - 16 bits inline */
  uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);
#else
/* Mode 3 - derived from server link-local (MAC) address */
  uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0x0250, 0xc2ff, 0xfea8, 0xcd1a); //redbee-econotag
#endif
}
/*---------------------------------------------------------------------------*/
#ifndef BROADCAST_MODE            // this function sends NTP client message to REMOTE_HOST
static void
timeout_handler(void)
{
 ntp_client_send(client_conn,server_ipaddr,msg);
}
#endif
/*------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
PRINTF ("Recv a packet \n");
ntp_adjust_time();
}
/*---------------------------------------------------------------*/
PROCESS_THREAD(ntpd_process, ev, data)
{
  static struct etimer et;
  static struct etimer et_check_c;

  PROCESS_BEGIN();
  
  PROCESS_PAUSE();
 
  set_global_address();
  
  PRINTF("UDP client process started\n");

  print_local_addresses();

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
if (etimer_expired(&et_check_c))
{
unsigned long check_second;
rtimer_clock_t check_clock_counter;
check_second=clock_seconds();
check_clock_counter=clock_counter();
PRINTF ("current time: %lu,%lu \n",check_second,check_clock_counter);
etimer_reset(&et_check_c); 
}
  }

  PROCESS_END();
}
