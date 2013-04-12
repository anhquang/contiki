/*
 * Nguyen Quoc Dinh <nqdinh@iuh.edu.vn
 * Dec, 2-2012
 *
 */

#ifndef __MIB_CONSTANT_H__
#define	__MIB_CONSTANT_H__

/**** IF-MIB initialization functions ****************/
#define ifIndex 1
#define ifDescr 2
#define ifType	3
#define ifMtu	4
#define ifSpeed	5
#define ifPhysAddress	6


/* -------- ENTITY-MIB initialization functions --------------*/
#define entPhysicalIndex	1
#define entPhysicalDescr	2
#define entPhysicalVendorType	3
#define	entPhysicalContainedIn	4
#define entPhysicalClass		5
#define entPhysicalParentRelPos	6
#define entPhysicalName			7
#define entPhysicalHardwareRev	8
#define entPhysicalFirmwareRev	9
#define	entPhysicalSoftwareRev	10
#define entPhysicalSerialNum	11
#define	entPhysicalMfgName		12
#define entPhysicalModelName	13
#define entPhysicalAlias		14
#define	entPhysicalAssetID		15
#define entPhysicalIsFRU		16
#define entPhysicalMfgDate		17
#define entPhysicalUris			18


#define PHYCLASS_OTHER		1
#define	PHYCLASS_UNKNOWN		2
#define	PHYCLASS_CHASSIS		3
#define	PHYCLASS_BACKPLANE		4
#define	PHYCLASS_CONTAINER		5
#define	PHYCLASS_POWERSUPPLY	6
#define	PHYCLASS_FAN		7
#define	PHYCLASS_SENSOR		8
#define	PHYCLASS_MODULE		9
#define	PHYCLASS_PORT		10
#define	PHYCLASS_STACK		11
#define	PHYCLASS_CPU		12

/* -------- ENTITY-SENSOR_MIB initialization functions --------------*/
#define entPhySensorType	1
#define entPhySensorScale	2
#define entPhySensorPrecision	3
#define entPhySensorValue		4
#define entPhySensorOperStatus	5
#define entPhySensorUnitsDisplay	6
#define	entPhySensorValueTimeStamp	7
#define entPhySensorValueUpdateRate	8

#define SENSORDATATYPE_OTHER	1
#define SENSORDATATYPE_UNKNOWN	2
#define SENSORDATATYPE_VOLTSAC	3
#define SENSORDATATYPE_VOLTDC	4
#define SENSORDATATYPE_AMPERES	5
#define SENSORDATATYPE_WATTS	6
#define SENSORDATATYPE_HERTZ	7
#define SENSORDATATYPE_CELSIUS	8
#define SENSORDATATYPE_PERCENTRH	9
#define SENSORDATATYPE_RPM		10
#define SENSORDATATYPE_CMM		11
#define SENSORDATATYPE_TRUTHVALUE	12

#define SENSORDATASCALE_YOCTO	1
#define SENSORDATASCALE_ZEPTO	2
#define SENSORDATASCALE_ATTO	3
#define SENSORDATASCALE_FEMTO	4
#define SENSORDATASCALE_PICO	5
#define SENSORDATASCALE_NANO	6
#define SENSORDATASCALE_MICRO	7
#define SENSORDATASCALE_MILLI	8
#define SENSORDATASCALE_UNIT	9
#define SENSORDATASCALE_KILO	10
#define SENSORDATASCALE_MEGA	11
#define SENSORDATASCALE_GIGA	12
#define SENSORDATASCALE_TERA	13
#define SENSORDATASCALE_EXA		14
#define SENSORDATASCALE_PETA	15
#define SENSORDATASCALE_ZETTA	16
#define SENSORDATASCALE_YOTTA	17

#define SENSORSTATUS_OK		1
#define SENSORSTATUS_UNAVAILABLE	2
#define SENSORSTATUS_NONOPERATIONAL	3

#endif
