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

#include <stdlib.h>


#include "contiki-lib.h"
#include "contiki-net.h"

#include "ntpd.h"

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

/* Change ports to non-standard values - NTP_PORT is defined in ntpd.h */


/* NTP client uses clock_set_time if the local clock offset is
 * equal or greater than ADJUST_THRESHOLD seconds.
 */


/* If remote host is defined, assuming NTP unicast mode.
 * Client is active and sends ntp_msg to NTP server, otherwise no ntp_msg is needed.
 */

/* NTP Poll interval in seconds = 2^TAU
 * In NTPv4 TAU ranges from 4 (Poll interval 16 s) to 17 ( Poll interval 36 h)
 */
/// CAUTION: etimer is limited in Contiki to cca 500s (platform specific)
/// so do not use TAU > 8



void clock_set_time(unsigned long sec,unsigned long nsec)
{
clock_time_t cnt,tarcnt;
//boottime = sec - clock_seconds();
clock_set_seconds(sec);
cnt=sec*CLOCK_SECOND + nsec * (CLOCK_SECOND / 1000000000);
tarcnt=sec*(CLOCK_SECOND*256)+nsec * (CLOCK_SECOND*256)/1000000000;
clock_set(cnt,tarcnt);
}
//****//
void clock_get_time(struct time_spec *ts)
{
rtimer_clock_t tarcounter;
clock_time_t tmp_scount;
do {
//	ts->sec = boottime + clock_seconds();
	ts->sec = clock_seconds();
	do {
		tarcounter = clock_counter();
		tmp_scount = clock_time();
	} while  (tarcounter != clock_counter());
	
ts->nsec = tmp_scount * (1000000000 / CLOCK_SECOND)+tarcounter*(1000000000/(CLOCK_SECOND*256));

}
//while (ts->sec != (boottime+clock_seconds()));
while (ts->sec != (clock_seconds()));
}
//*****//
void clock_adjust_time(struct time_spec *delta)
{
if (delta->sec == 0L){
	if (delta->nsec == 0L){
	adjcompare=0;
	return;
	} else{
	adjcompare = -delta->nsec / (1000000000 / (CLOCK_SECOND*256));
}
}else	{
adjcompare=-delta->sec * (CLOCK_SECOND * 256)+-delta->nsec / (1000000000/(CLOCK_SECOND*256));
}
if (adjcompare ==0)
{
//do nothing//
}
else if (adjcompare > 0)
{
adjcompare--;
clock_inc_dec(adjcompare);
}
else
{
adjcompare++;
clock_inc_dec(adjcompare);
}


}
/*---------------------------------------------------------------------------*/
void ntp_server_send(struct uip_udp_conn *udpconn)
{
struct ntp_msg *pkt;
#ifndef BROADCAST_MODE
struct time_spec rects;
#endif
struct time_spec xmtts;

struct ntp_msg server_msg;
#ifndef BROADCAST_MODE
 if(uip_newdata()) {
    // get receive timestamp (t2)
   clock_get_time(&rects);

    // check if received packet is complete
    if((uip_datalen() != NTP_MSGSIZE_NOAUTH)
       && (uip_datalen() != NTP_MSGSIZE)) {
      PRINTF("Received malformed NTP packet\n");
      return;
    }
    PRINTF ("Received correct packet !!! \n");
    pkt = uip_appdata;

	server_msg.status = MODE_SERVER | (NTP_VERSION << 3) | LI_ALARM;
	server_msg.orgtime.int_partl=pkt->xmttime.int_partl;
	server_msg.orgtime.fractionl=pkt->xmttime.fractionl;
	server_msg.rectime.int_partl=uip_htonl(rects.sec + JAN_1970);
	server_msg.rectime.fractionl=uip_htonl(rects.nsec);
	clock_get_time(&xmtts);
 	server_msg.xmttime.int_partl = uip_htonl(xmtts.sec + JAN_1970);
  	server_msg.xmttime.fractionl = uip_htonl(xmtts.nsec);
  	
 send_unicast(udpconn,&UIP_IP_BUF->srcipaddr,&server_msg,sizeof (struct ntp_msg));
    //uip_ipaddr_copy(&udpconn->ripaddr, &UIP_IP_BUF->srcipaddr);
    //uip_udp_packet_send(udpconn, &server_msg, sizeof(struct ntp_msg));
    //uip_create_unspecified(&udpconn->ripaddr);
}
#else
server_msg.status = MODE_BROADCAST | (NTP_VERSION << 3) | LI_ALARM;
	
	clock_get_time(&xmtts);
 	server_msg.xmttime.int_partl = uip_htonl(xmtts.sec + JAN_1970);
  	server_msg.xmttime.fractionl = uip_htonl(xmtts.nsec);
  	
send_broadcast(udpconn, &server_msg, sizeof(struct ntp_msg));
#endif
}
/*---------------------------------------------------------------------------*/
void ntp_client_send(struct uip_udp_conn *udpconn, uip_ipaddr_t srv_ipaddr, struct ntp_msg clientmsg)
{
 clientmsg.status = MODE_CLIENT | (NTP_VERSION << 3) | LI_ALARM;
  
  clock_get_time(&ts);
  clientmsg.xmttime.int_partl = uip_htonl(ts.sec + JAN_1970);
  clientmsg.xmttime.fractionl = uip_htonl(ts.nsec);

uip_udp_packet_sendto(udpconn, &clientmsg, sizeof(struct ntp_msg),
                        &srv_ipaddr, UIP_HTONS(SERVER_PORT));
  PRINTF("Sent timestamp: %ld sec %ld nsec to ", ts.sec, ts.nsec);
#ifdef UIP_CONF_IPV6
  PRINT6ADDR(&udpconn->ripaddr);
#else
  PRINTLLADDR(&udpconn->ripaddr);
#endif /* UIP_CONF_IPV6 */
  PRINTF("\n");
}

/*---------------------------------------------------------------------------*/
void ntp_adjust_time()
{
  struct ntp_msg *pkt;          // pointer to incomming packet
  /* timestamps for offset calculation */
  // t1 == ts
//#ifdef REMOTE_HOST              // variables needed only for NTP unicast mode
  struct time_spec rects;       // t2

  struct time_spec xmtts;       // t3
//#endif /* REMOTE_HOST */
  struct time_spec dstts;       // t4

  /* timestamp for local clock adjustment */
  struct time_spec adjts;

  if(uip_newdata()) {
    // get destination (t4) timestamp
    clock_get_time(&dstts);

    // check if received packet is complete
    if((uip_datalen() != NTP_MSGSIZE_NOAUTH)
       && (uip_datalen() != NTP_MSGSIZE)) {
      PRINTF("Received malformed NTP packet\n");
      return;
    }

    pkt = uip_appdata;

    // check if the server is synchronised
/*#if 0                           // change to 1 for strict check
    if(((pkt->status & LI_ALARM) == LI_ALARM)
       || (pkt->stratum > NTP_MAXSTRATUM) || (pkt->stratum == 0)
       || ((pkt->xmttime.int_partl) == (uint32_t) 0))
#else*/
    if((pkt->stratum > NTP_MAXSTRATUM)
       || (pkt->xmttime.int_partl) == (uint32_t) 0)
//#endif
    {
      PRINTF("Received NTP packet from unsynchronised server\n");
      return;
    }

    /* Compute adjustment */
    if((pkt->status & MODEMASK) == MODE_BROADCAST)      // in broadcast mode compute time from xmt and dst
    {
      // local clock offset THETA = t3 - t4
      adjts.sec = (uip_ntohl(pkt->xmttime.int_partl) - JAN_1970) - dstts.sec;
      adjts.nsec =
        fractionl_to_nsec(uip_htonl(pkt->xmttime.fractionl)) - dstts.nsec;
    }
#ifdef BROADCAST_MODE             // if only NTP broadcast mode supported
    else                        // in broadcast only mode, no other calcualtion is possible
    {
      PRINTF("Received NTP non-broadcast mode message\n");
      return;
    }
#else
    else                        // in client-server mode calculate local clock offset
    {
      if(ts.sec != (uip_ntohl(pkt->orgtime.int_partl) - JAN_1970)) {
        PRINTF
          ("Orgtime mismatch between received NTP packet and timestamp sent by us\n");
        return;
      }

      /* Compute local clock offset THETA = ((t2 - t1) + (t3 - t4)) / 2
       * for seconds part
       */
      rects.sec = uip_htonl(pkt->rectime.int_partl) - JAN_1970;
      xmtts.sec = uip_htonl(pkt->xmttime.int_partl) - JAN_1970;
      adjts.sec = ((rects.sec - ts.sec) + (xmtts.sec - dstts.sec)) / 2;

      /* Compute local clock offset for fraction part */
      rects.nsec = fractionl_to_nsec(uip_htonl(pkt->rectime.fractionl));
      xmtts.nsec = fractionl_to_nsec(uip_htonl(pkt->xmttime.fractionl));

      /* Correct fraction parts if seconds are adjacent */
      if(adjts.sec == 0) {
        if(ts.sec < rects.sec)  // server received packet in other second
        {
          ts.nsec -= 1000000000;
        }
        if(xmtts.sec < dstts.sec)       // our client received packet in other second
        {
          dstts.nsec += 1000000000;
        }
      }

      adjts.nsec = ((rects.nsec - ts.nsec) + (xmtts.nsec - dstts.nsec)) / 2;
    }

    /* Set our timestamp to zero to avoid processing the same packet more than once */
    ts.sec = 0;
#endif 
#ifdef DEBUG                    // correct plus minus for printing
    if((adjts.sec < 0) && (adjts.nsec > 0)) {
      adjts.sec = adjts.sec + 1;
      adjts.nsec = adjts.nsec - 1000000000;
    } else if((adjts.sec > 0) && (adjts.nsec < 0)) {
      adjts.sec = adjts.sec - 1;
      adjts.nsec = adjts.nsec + 1000000000;
    }
#endif

    PRINTF("Local clock offset = %ld sec %ld nsec\n", adjts.sec, adjts.nsec);

    /* Set or adjust local clock */
    if((adjts.sec) >= ADJUST_THRESHOLD) {
      PRINTF("Setting the time to xmttime from server\n");
      clock_set_time(uip_ntohl(pkt->xmttime.int_partl) - JAN_1970,uip_ntohl(pkt->xmttime.fractionl));
    } else {
      PRINTF("Adjusting the time for %ld and %ld\n", adjts.sec, adjts.nsec);
      clock_adjust_time(&adjts);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
send_unicast(struct uip_udp_conn *udpconn, uip_ipaddr_t *dest, struct ntp_msg *buf, int size)
{
  uip_ipaddr_copy(&udpconn->ripaddr, dest);
  uip_udp_packet_send(udpconn, buf, size);
  uip_create_unspecified(&udpconn->ripaddr);
}

void
send_broadcast(struct uip_udp_conn *udpconn,struct ntp_msg *buf, int size)
{
  uip_create_linklocal_allnodes_mcast(&udpconn->ripaddr);
  uip_udp_packet_send(udpconn, buf, size);
  uip_create_unspecified(&udpconn->ripaddr);
}
