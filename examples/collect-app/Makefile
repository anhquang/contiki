CONTIKI_PROJECT=collectd-app
all: $(CONTIKI_PROJECT) 

UIP_CONF_IPV6=1
WITH_UIP6=1

CFLAGS+= -DUIP_CONF_IPV6_RPL
CFLAGS+= -DPROJECT_CONF_H=\"project-conf.h\"

APPS+=collectd
#APPS+=snmpd

CONTIKI=../..
include $(CONTIKI)/Makefile.include

SMALL=1
