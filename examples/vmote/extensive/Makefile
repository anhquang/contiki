PROJECT=vmote-mote

UIP_CONF_IPV6=1
WITH_UIP6=1

APPS+=snmpd 
APPS+=collectd

CONTIKI=../../..
include $(CONTIKI)/Makefile.include

SMALL=1

CFLAGS += -ffunction-sections

LDFLAGS += -Wl,--gc-sections,--undefined=_reset_vector__,--undefined=InterruptVectors,--undefined=_copy_data_init__,--undefined=_clear_bss_init__,--undefined=_end_of_init__

#CFLAGS   += -DENABLE_SNMPv1
#CFLAGS   += -DENABLE_SNMPv3

vmote: 
	make TARGET=vmote MIB_INIT=mib-init.c vmote-mote

vmote-upload: 
	make TARGET=vmote MIB_INIT=mib-init.c vmote-mote.vmote.u AVRDUDE_PORT=/dev/ttyUSB0
	avr-size -C --mcu=atmega128rfa1 $(PROJECT).vmote

128rfa1:
	make TARGET=avr-atmega128rfa1 MIB_INIT=mib-init.c $(PROJECT).elf
	avr-size -C --mcu=atmega128rfa1 $(PROJECT).elf

