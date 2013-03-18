PROJECT=collectd-app

UIP_CONF_IPV6=1
WITH_UIP6=1

CFLAGS+= -DUIP_CONF_IPV6_RPL

APPS+=collectd
#APPS+=snmpd

CONTIKI=../..
include $(CONTIKI)/Makefile.include

SMALL=1
sky:
	make TARGET=sky collectd-app
