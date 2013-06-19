CONTIKI_PROJECT=collectd-app
all: $(CONTIKI_PROJECT) 

UIP_CONF_IPV6=1
WITH_UIP6=1

CFLAGS+= -DUIP_CONF_IPV6_RPL
CFLAGS+= -DUIP_CONF_BUFFER_SIZE=300
ifeq ($(TARGET), sky)
CFLAGS+= -DPROJECT_CONF_H=\"project-conf.h\"
endif

APPS+=man
#APPS+=snmpd

CONTIKI=../..
include $(CONTIKI)/Makefile.include

SMALL=1

vmote-upload: 
	make TARGET=vmote $(CONTIKI_PROJECT).vmote.u
	avr-size -C --mcu=atmega128rfa1 $(CONTIKI_PROJECT).vmote
