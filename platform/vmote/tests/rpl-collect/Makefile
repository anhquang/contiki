CONTIKI = ../../../..
APPS = powertrace collect-view
CONTIKI_PROJECT = udp-sender udp-sink
PROJECT_SOURCEFILES += collect-common.c

WITH_UIP6=1
UIP_CONF_IPV6=1
TARGET=vmote
CFLAGS += -DNODE=$(NODE)
all: $(CONTIKI_PROJECT)

include $(CONTIKI)/Makefile.include

