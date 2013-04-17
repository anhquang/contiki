static void
tcpip_handler(void)
{
struct ntp_msg *pkt;
struct time_spec rects;
struct time_spec xmtts;
 if(uip_newdata()) {
    // get receive timestamp (t2)
    clock_get_time(&rects);

    // check if received packet is complete
    if((uip_datalen() != NTP_MSGSIZE_NOAUTH)
       && (uip_datalen() != NTP_MSGSIZE)) {
      PRINTF("Received malformed NTP packet\n");
      return;
    }

    pkt = uip_appdata;
	server_msg.status = MODE_SERVER | (NTP_VERSION << 3) | LI_ALARM;
	server_msg.orgtime.int_partl=pkt->xmttime.int_partl;
	server_msg.orgtime.fractionl=pkt->xmttime.fractionl;
	server_msg.rectime.int_partl=uip_htonl(rects.sec + JAN_1970);
	server_msg.rectime.fractionl=uip_htonl(rects.nsec);
	clock_get_time(&xmtts);
 	server_msg.xmttime.int_partl = uip_htonl(xmtts.sec + JAN_1970);
  	server_msg.xmttime.fractionl = uip_htonl(xmtts.nsec);
  	uip_udp_packet_send(server_conn, &server_msg, sizeof(struct ntp_msg));


}
