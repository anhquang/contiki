all: ntp-client ntp-server
APPS = ntpd-sky servreg-hack
CONTIKI=../..

WITH_UIP6=1
UIP_CONF_IPV6=1
CC2420_CONF_AUTOACK=0
CFLAGS+=-D'BROADCAST_MODE="1"'
CFLAGS+= -DUIP_CONF_IPV6_RPL
#CFLAGS+=-DMAC_ADDRESS="{0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, 0x44, 0x55}"
#CFLAGS+=-D'REMOTE_HOST="aaaa::1"'

include $(CONTIKI)/Makefile.include
