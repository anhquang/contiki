#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "jsmn.h"

#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

/* Terminate current test with error */
#define fail()	return __LINE__
/* Successfull end of the test case */
#define done() return 0
/* Check single condition */
#define check(cond) do { if (!(cond)) fail(); } while (0)

#define TOKEN_EQ(t, tok_start, tok_end, tok_type) \
	((t).start == tok_start \
	 && (t).end == tok_end  \
	 && (t).type == (tok_type))
#define TOKEN_STRING(js, t, s) \
	(strncmp(js+(t).start, s, (t).end - (t).start) == 0 \
	 && strlen(s) == (t).end - (t).start)
#define TOKEN_PRINT(t) \
	printf("start: %d, end: %d, type: %d, size: %d\n", \
			(t).start, (t).end, (t).type, (t).size)

#define SEND_INTERVAL		15 * CLOCK_SECOND
#define MAX_PAYLOAD_LEN		40
#define MAX_UDP_LEN			200

static struct uip_udp_conn *client_conn;
static struct uip_udp_conn *server_conn;
/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  char *str;

  if(uip_newdata()) {
    str = uip_appdata;
    str[uip_datalen()] = '\0';
    printf("Response from the server: '%s'\n", str);
  }
}
/*---------------------------------------------------------------------------*/
static char buf[MAX_PAYLOAD_LEN];
static char json[MAX_UDP_LEN];
static short clk(void)
{
	return 31;
}
static short syn(void)
{
	return 40;
}
static short cpu(void)
{
	return 50;
}
static void timeout_handler(void)
{
  static int seq_id;

  printf("Client sending to: ");
  PRINT6ADDR(&client_conn->ripaddr);
  sprintf(buf, "Hello %d from the client", ++seq_id);
  printf(" (msg: %s)\n", buf);

  sprintf(json, "{\'clk\':%d,\'syn\': %d,\'cpu\':%d}", clk(), syn(), cpu());

#if SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION
  uip_udp_packet_send(client_conn, buf, UIP_APPDATA_SIZE);
#elif 0 /* SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION */
  uip_udp_packet_send(client_conn, buf, strlen(buf));
#else
  uip_udp_packet_send(client_conn, json, strlen(json));
#endif /* SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION */
}

/*---------------------------------------------------------------------------*/
int test_simple() {
	const char *js;
	int r;
	jsmn_parser p;
	jsmntok_t tokens[10];
    uip_ipaddr_t ipaddr;

    char val[120];

	js = "{\"addr\": \"aaaa:2::1:1\"}";

	jsmn_init(&p);
	r = jsmn_parse(&p, js, tokens, 10);
	check(r == JSMN_SUCCESS);

	check(TOKEN_STRING(js, tokens[0], js));

    token_value_get(js, tokens[2], val);
    printf("value of \"a\" is %s\n", val);

    /* convert to real addr */
    if(uiplib_ipaddrconv(val, &ipaddr) == 0) {
        printf("error");
    }
    printf("server ip (after convert) is ");
    uip_debug_ipaddr_print(&ipaddr);
    printf("\n");

	return 0;
}

/*---------------------------------------------------------------------------*/
static void
set_connection_address(uip_ipaddr_t *ipaddr)
{

  uip_ip6addr(ipaddr,0xaaaa,0,0,0,0,0,0,0x1);

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer et;
  uip_ipaddr_t ipaddr;
  static int i;

  PROCESS_BEGIN();
  PRINTF("UDP client process started\n");

  set_connection_address(&ipaddr);

  /* new connection with remote host */
  client_conn = udp_new(&ipaddr, UIP_HTONS(3000), NULL);
  udp_bind(client_conn, UIP_HTONS(3001));

  /* new connection with remote host */
  	server_conn = udp_new(NULL, UIP_HTONS(0), NULL);
  	udp_bind(server_conn, UIP_HTONS(2000));

    //testjson
    i = test_simple();
    printf("result = %d", i);

  PRINTF("Created a connection with the server ");
  PRINT6ADDR(&client_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
	UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));

  etimer_set(&et, SEND_INTERVAL);
  while(1) {
    PROCESS_YIELD();
    if(etimer_expired(&et)) {
      //timeout_handler();
      etimer_restart(&et);
    } else if(ev == tcpip_event) {
      tcpip_handler();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
