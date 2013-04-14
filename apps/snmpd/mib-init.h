/* -----------------------------------------------------------------------------
 * SNMP implementation for Contiki
 *
 * Copyright (C) 2010 Siarhei Kuryla
 *
 * This program is part of free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

/**
 * \file
 *         MIB object initialization.
 * \author
 *         Siarhei Kuryla <kurilo@gmail.com>
 */

#ifndef __MIBINIT_H__
#define	__MIBINIT_H__

#include <stdlib.h>
#include "mib.h"
#include "mib-constant.h"

#if CONTIKI_TARGET_SKY
#define SNMP_SYSDESCR "ubisen, sky, Contiki 2.6"
#elif CONTIKI_TARGET_VMOTE || CONTIKI_TARGET_ZIGD
#define SNMP_SYSDESCR "ubisen, vmote 1.0, Contiki 2.6"
#else
#define SNMP_SYSDESCR "Undefined HW, Contiki 2.6"
#endif
#define SNMP_SYSNAME "@ubisen ltd vn"
#define SNMP_SYSUPTIME	1234
#define SNMP_IFDESCR	"lwpan"
#define SNMP_IANAIFTYPE_MIB_IEEE802154 	259
#define SNMP_MTU_IEEE802154				127
#define SNMP_SPEED_IEEE802154			250000

#define IFNUMBER 	1					//by default, there is only one connection on each mote
#define IFENTRYMAX 	ifPhysAddress

#define PHYSICALNUMBER		3		//chassis, cpu, port, sensor x3 (temper + rssi + kwh meter)
#define PHYSICALENTRYMAX 		entPhysicalClass
#define ENT_PHYSICAL_VENDOR_TYPE_VALUE 	{0, 0}	//i have no registration for this mote id, hence, the default value would be {0 0}
#define ENT_PHYSICAL_CLASS_VALUE 	{PHYCLASS_CHASSIS, PHYCLASS_CPU, PHYCLASS_PORT, PHYCLASS_SENSOR, PHYCLASS_SENSOR, PHYCLASS_SENSOR}
#define	ENT_PHYSICAL_DESCR_VALUE	{"Sky", "MSP430", "802.15.4 NI", "Temp Sensor", "802.15.4 RSSI Sensor", "kWh Meter"}


#define SENSORNUMBER 		4				//have 3 sensors: (temperature + rssi + kwh meter). See ENTITY-MIB
#define SENSORENTRYMAX 		entPhySensorValueUpdateRate

#define ENT_PHYSENSOR_TYPE_VALUE		{SENSORDATATYPE_CELSIUS, SENSORDATATYPE_OTHER, SENSORDATATYPE_WATTS, SENSORDATATYPE_WATTS}
#define ENT_PHYSENSOR_SCALE_VALUE		{SENSORDATASCALE_UNIT, SENSORDATASCALE_UNIT, SENSORDATASCALE_UNIT, SENSORDATASCALE_UNIT}
#define ENT_PHYSENSOR_PRECISION_VALUE 	{0, 0, 0, 0}		//value range from [-8, 9]
#define ENT_PHYSENSOR_OPER_STATUS_VALUE			{SENSORSTATUS_OK, SENSORSTATUS_OK, SENSORSTATUS_OK, SENSORSTATUS_OK}
#define ENT_PHYSENSOR_UNITS_DISPLAY_VALUE		{"Cel", "dB", "W", "W"}
#define ENT_PHYSENSOR_VALUE_TIMESTAMP_VALUE		{1234, 1234, 1234, 1234}
#define ENT_PHYSENSOR_VALUE_UPDATE_RATE_VALUE	{0, 0, 0, 0};	//in milliseconds

/* system name structure, the snmp object could be read and write */
#define SYSNAME_LEN		20
struct snmp_sysname_t {
	u8t sysname[SYSNAME_LEN];
	u8t sysnamelen;
};
#define SYSNAME_FS_FILENAME "snmp_sysname"

/**
 * Adds all necessary object to the MIB. Should be changed if a new object needs to be added to the MIB.
 * \brief Initializes the MIB objects.
 */
s8t mib_init();

#define SIMULATION			1
#endif	/* __MIBINIT_H__ */

