# make upload AVRDUDE_PORT=/dev/ttyUSB1 NODE=3
# make login AVRDUDE_PORT=/dev/ttyUSB1
#
CONTIKI_PROJECT = ping-ipv6
all: $(CONTIKI_PROJECT)
CONTIKI_TARGET_SOURCEFILES += ping6.c 
CFLAGS += -DNODE=$(NODE)
UIP_CONF_IPV6=1
TARGET=zigd
CONTIKI = ../../../..
include $(CONTIKI)/Makefile.include
