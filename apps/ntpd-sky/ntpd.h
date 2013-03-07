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

#ifndef __NTPD_H__
#define __NTPD_H__

#include <inttypes.h>

#include "contiki.h"
#include "sys/clock.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-udp-packet.h"
/*
 * Numbers and conversions according to RFC 5905
 */
#define NTP_PORT             123
#define	JAN_1970             2208988800UL    /* 1970 - 1900 in seconds */
#define ADJUST_THRESHOLD 0
#define	NTP_VERSION	4
#define	NTP_MAXSTRATUM	15
#define CLIENT_PORT NTP_PORT
#define SERVER_PORT NTP_PORT
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
//static struct uip_udp_conn *udpconn;
/*
 * NTP Packet sizes
 */
#define	NTP_DIGESTSIZE		16
#define	NTP_MSGSIZE_NOAUTH	48
#define	NTP_MSGSIZE		(NTP_MSGSIZE_NOAUTH + 4 + NTP_DIGESTSIZE)
//#ifdef REMOTE_HOST
struct time_spec ts;
#ifndef TAU
#define TAU 4
#endif /* TAU */

//#else
//#warning "No REMOTE_HOST defined - only NTP broadcast messages will be processed!"
//#endif /* REMOTE_HOST */
/*
 * NTP structures
 */

static volatile int16_t adjcompare;


struct l_fixedpt {
	uint32_t int_partl;
	uint32_t fractionl;
};

struct s_fixedpt {
	uint16_t int_parts;
	uint16_t fractions;
};

/*
 * NTP Packet structure - we are not dealing with digest
 */
struct ntp_msg {
	uint8_t status; /* status of local clock and leap info */
	uint8_t stratum;	/* Stratum level */
	uint8_t ppoll;		/* poll value */
	int8_t precision;
	struct s_fixedpt rootdelay;
	struct s_fixedpt dispersion;
	uint32_t refid;
	struct l_fixedpt reftime;
	struct l_fixedpt orgtime;
	struct l_fixedpt rectime;
	struct l_fixedpt xmttime;
};


/*
 * Macros for packet
 */

/* Leap Second Codes (high order two bits) */
#define	LI_NOWARNING	(0 << 6)	/* no warning */
#define	LI_PLUSSEC	(1 << 6)	/* add a second (61 seconds) */
#define	LI_MINUSSEC	(2 << 6)	/* minus a second (59 seconds) */
#define	LI_ALARM	(3 << 6)	/* alarm condition */

/* Status Masks */
#define	MODEMASK	(7 << 0)
#define	VERSIONMASK	(7 << 3)
#define LIMASK		(3 << 6)

/* Mode values */
#define	MODE_RES0	0	/* reserved */
#define	MODE_SYM_ACT	1	/* symmetric active */
#define	MODE_SYM_PAS	2	/* symmetric passive */
#define	MODE_CLIENT	3	/* client */
#define	MODE_SERVER	4	/* server */
#define	MODE_BROADCAST	5	/* broadcast */
#define	MODE_RES1	6	/* reserved for NTP control message */
#define	MODE_RES2	7	/* reserved for private use */


/*
 * Convert functions between NTP and local timestamp
 */
void ntp_to_ts(const struct l_fixedpt *ntp, struct time_spec *ts);
unsigned long fractionl_to_nsec(uint32_t fractionl);
void clock_set_time(unsigned long sec,unsigned long nsec);
void clock_adjust_time(struct time_spec *delta);
void clock_get_time(struct time_spec *ts);
void ntp_server_send(struct uip_udp_conn *udpconn);
void ntp_client_send(struct uip_udp_conn *udpconn,uip_ipaddr_t srv_ipaddr,struct ntp_msg clientmsg);
void ntp_adjust_time();
void
send_unicast(struct uip_udp_conn *udpconn, uip_ipaddr_t *dest, struct ntp_msg *buf, int size);
void
send_broadcast(struct uip_udp_conn *udpconn,struct ntp_msg *buf, int size);
#endif /* __NTPD_H__ */
