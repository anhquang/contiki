all: udp-server udp-client
TARGET = vmote
UIP_CONF_IPV6 = 1
CFLAGS += -DUDP_CONNECTION_ADDR=fdfd::0000:0000:0000:0011:22ff:fe33:440$(NODE)
CONTIKI = ../../../..
include $(CONTIKI)/Makefile.include
