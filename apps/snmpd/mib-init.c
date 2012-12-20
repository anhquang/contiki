#include <stdlib.h>

#include "mib-init.h"
#include "ber.h"
#include "utils.h"
#include "logging.h"

#include "net/rime.h"

#if CONTIKI_TARGET_AVR_RAVEN && ENABLE_PROGMEM
#include <avr/pgmspace.h>
#else
#define PROGMEM
#endif

/* SNMPv2-MIB
 * just give out some parameters
 * for the commented oid, just ignored at the moment
 * */
static u8t ber_oid_system_desc[] PROGMEM  = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x01, 0x01, 0x00};
static ptr_t oid_system_desc PROGMEM      = {ber_oid_system_desc, 8};
/*
static u8t ber_oid_system_objectid[] PROGMEM  = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x01, 0x02, 0x00};
static ptr_t oid_system_objectid PROGMEM      = {ber_oid_system_objectid, 8};
*/
static u8t ber_oid_system_time[] PROGMEM  = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x01, 0x03, 0x00};
static ptr_t oid_system_time PROGMEM      = {ber_oid_system_time, 8};
/*
static u8t ber_oid_system_contact[] PROGMEM  = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x01, 0x04, 0x00};
static ptr_t oid_system_contact PROGMEM      = {ber_oid_system_contact, 8};
*/
static u8t ber_oid_system_sysname[] PROGMEM = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x01, 0x05, 0x00};
static ptr_t oid_system_system PROGMEM 		= {ber_oid_system_sysname, 8};
/*
static u8t ber_oid_system_location[] PROGMEM  = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x01, 0x06, 0x00};
static ptr_t oid_system_location PROGMEM      = {ber_oid_system_location, 8};

static u8t ber_oid_system_services[] PROGMEM  = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x01, 0x07, 0x00};
static ptr_t oid_system_services PROGMEM      = {ber_oid_system_services, 8};

static u8t ber_oid_system_or_lastchange[] PROGMEM  = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x01, 0x08, 0x00};
static ptr_t oid_system_or_lastchange PROGMEM      = {ber_oid_system_or_lastchange, 8};

static u8t ber_oid_system_ortable[] PROGMEM  = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x01, 0x09, 0x00};
static ptr_t oid_system_ortable PROGMEM      = {ber_oid_system_ortable, 8};
*/


/*
 * IF-MIB
 * NOTE: I change iftable into table, not scala anymore.
 */
static u8t ber_oid_if_number[] PROGMEM    = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x02, 0x01, 0x00};
static ptr_t oid_if_number PROGMEM        = {ber_oid_if_number, 8};

static u8t ber_oid_if_table[] PROGMEM     = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x02, 0x02, 0x01};
static ptr_t oid_if_table PROGMEM         = {ber_oid_if_table, 8};

/*
 * IP-MIB
 * both ipv4 and ipv6
 * however, i pay attention to v6 only
 */
/*
static u8t ber_oid_ipAddress_ifIndex[] PROGMEM	= {};
static ptr_t oid_ipAddress_ifindex				= {ber_oid_ipAddress_ifIndex, 10};

static u8t ber_oid_ipAddress_ifIndex[] PROGMEM	= {};
static ptr_t oid_ipAddress_ifindex				= {ber_oid_ipAddress_ifIndex, 10};

static u8t ber_oid_ipAddress_ifIndex[] PROGMEM	= {};
static ptr_t oid_ipAddress_ifindex				= {ber_oid_ipAddress_ifIndex, 10};

static u8t ber_oid_ipAddress_ifIndex[] PROGMEM	= {};
static ptr_t oid_ipAddress_ifindex				= {ber_oid_ipAddress_ifIndex, 10};

static u8t ber_oid_ipAdEntAddr[] PROGMEM  = {0x2b, 6, 1, 2, 1, 4, 20, 1, 1};
static ptr_t oid_ipAdEntAddr PROGMEM	  = {ber_oid_ipAdEntAddr, 9};

static u8t ber_oid_test_int[] PROGMEM     = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x89, 0x52, 0x01, 0x00};
static ptr_t oid_test_int PROGMEM         = {ber_oid_test_int, 9};
static u8t ber_oid_test_uint[] PROGMEM    = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x89, 0x52, 0x02, 0x00};
static ptr_t oid_test_uint PROGMEM        = {ber_oid_test_uint, 9};
*/

/*
 * ENTITY-MIB
 * entityPhysical (1.3.6.1.2.1.47.1.1.1.1) table only, don't have entityLogical yet
 */
static u8t ber_oid_entPhysicalEntry[] PROGMEM     = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x2f, 0x01, 0x01, 0x01, 0x01};
static ptr_t oid_entPhysicalEntry PROGMEM         = {ber_oid_entPhysicalEntry, 10};

/*
 * ENTITY-SENSOR-MIB
 * entPhySensorEntry (1.3.6.1.2.1.99.1.1.1) table only
 */
static u8t ber_oid_entPhySensorEntry[] PROGMEM     = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x63, 0x01, 0x01, 0x01};
static ptr_t oid_entPhySensorEntry PROGMEM         = {ber_oid_entPhySensorEntry, 9};


/**** SNMPv2-MIB initialization functions ****************/
s8t getSysDescr(mib_object_t* object, u8t* oid, u8t len)
{
    if (!object->varbind.value.p_value.len) {
        object->varbind.value.p_value.ptr = (u8t*)"Contiki SNMP";
        object->varbind.value.p_value.len = strlen("Contiki SNMP");
    }
    return 0;
}

s8t setSysDescr(mib_object_t* object, u8t* oid, u8t len, varbind_value_t value)
{
    object->varbind.value.p_value.ptr = (u8t*)"System Description2";
    object->varbind.value.p_value.len = 19;
    return 0;
}

s8t getTimeTicks(mib_object_t* object, u8t* oid, u8t len)
{
    object->varbind.value.u_value = 1234;
    return 0;
}

/**** IF-MIB initialization functions ****************/
#define IFNUMBER 1					//by default, there is only one connection on each mote
#define ifIndex 1
#define ifDescr 2
#define ifType	3
#define ifMtu	4
#define ifSpeed	5
#define ifPhysAddress	6
#define IFENTRYMAX ifPhysAddress

s8t getIfNumber(mib_object_t* object, u8t* oid, u8t len)
{
    object->varbind.value.i_value = IFNUMBER;
    return 0;
}

s8t getIf(mib_object_t* object, u8t* oid, u8t len)
{
    u32t oid_el1, oid_el2;
    u8t i;
    i = ber_decode_oid_item(oid, len, &oid_el1);
    i = ber_decode_oid_item(oid + i, len - i, &oid_el2);

    if (len != 2)
        return -1;

    if (!(0 < oid_el2 && oid_el2 <= IFNUMBER))
    	return -1;

    switch (oid_el1) {
		case ifIndex:
			object->varbind.value_type = BER_TYPE_INTEGER;
			object->varbind.value.i_value = oid_el2;

			break;
		case ifDescr:
			object->varbind.value_type = BER_TYPE_OCTET_STRING;
			object->varbind.value.p_value.ptr = (u8t*)"lwpan";
			object->varbind.value.p_value.len = strlen("lwpan");
			break;
        case ifType:
        	object->varbind.value_type = BER_TYPE_INTEGER;
        	object->varbind.value.i_value = IANAIFTYPE_MIB_IEEE802154;
        	break;
        case ifMtu:
        	object->varbind.value_type = BER_TYPE_INTEGER;
        	object->varbind.value.i_value = MTU_IEEE802154;			//ianaiftype-mib ieee802.15.4 type
        	break;
        case ifSpeed:
        	object->varbind.value_type = BER_TYPE_GAUGE;
        	object->varbind.value.u_value = SPEED_IEEE802154;
        	break;
        case ifPhysAddress:
        	object->varbind.value_type = BER_TYPE_OCTET_STRING;
			object->varbind.value.p_value.ptr = (u8t*)&rimeaddr_node_addr;
			//object->varbind.value.p_value.ptr = (u8t*)"MAC Add";
			object->varbind.value.p_value.len = 8;
        	break;
        default:
            break;
    }
    return 0;
}

ptr_t* getNextIfOid(mib_object_t* object, u8t* oid, u8t len)
{
    u32t oid_el1, oid_el2;
    u8t i;
    i = ber_decode_oid_item(oid, len, &oid_el1);
    i = ber_decode_oid_item(oid + i, len - i, &oid_el2);

    /* oid_el1 = [1..IFENTRYMAX]
     * oid_el2 = [1..IFNUMBER]
     */

    if (oid_el1 < IFENTRYMAX || (oid_el1 == IFENTRYMAX && oid_el2 < IFNUMBER)) {
        ptr_t* ret = oid_create();
        CHECK_PTR_U(ret);
        ret->len = 2;
        ret->ptr = malloc(2);
        CHECK_PTR_U(ret->ptr);
        if (oid_el2 < IFNUMBER) {
        	oid_el2++;
        } else if (oid_el2 >= IFNUMBER) {
        	if (oid_el1 < IFENTRYMAX) {
        		oid_el1++;
        		oid_el2 = 1;
        	} else {
        		return 0;
        	}
        }
        if (oid_el1 < 1) oid_el1 = 1;
        if (oid_el2 < 1) oid_el2 = 1;
        ret->ptr[0] = oid_el1;
        ret->ptr[1] = oid_el2;
        return ret;
    }
    return 0;
}
/*end IF-MIB initialization functions*/

/* -------- ENTITY-MIB initialization functions --------------*/
#define PHYSICALNUMBER		3		//chassis, cpu, port, sensor x3 (temper + rssi + kwh meter)
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
#define PHYSICALENTRYMAX 		entPhysicalClass

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

s8t getEntityPhysicalEntry(mib_object_t* object, u8t* oid, u8t len) {
    u32t oid_el1, oid_el2;
    u8t i;
    u8t entPhysicalVendorTypeValue[] PROGMEM = {0, 0};
    char entPhysicalClassValue[] PROGMEM ={PHYCLASS_CHASSIS, PHYCLASS_CPU, PHYCLASS_PORT, PHYCLASS_SENSOR, PHYCLASS_SENSOR, PHYCLASS_SENSOR};
    char *entPhysicalDescrValue[] PROGMEM = {"Sky", "MSP430", "802.15.4 NI", "Temp Sensor", "802.15.4 RSSI Sensor", "kWh Meter"};

    i = ber_decode_oid_item(oid, len, &oid_el1);
    i = ber_decode_oid_item(oid + i, len - i, &oid_el2);

    if (len != 2)
        return -1;

    if (!(0 < oid_el2 && oid_el2 <= PHYSICALNUMBER))
        return -1;

    switch (oid_el1) {
    case entPhysicalIndex:
    	object->varbind.value_type = BER_TYPE_INTEGER;
		object->varbind.value.i_value = oid_el2;

		break;
    case entPhysicalDescr:
    	object->varbind.value_type = BER_TYPE_OCTET_STRING;
		object->varbind.value.p_value.ptr = (char*)entPhysicalDescrValue[oid_el2-1];
		object->varbind.value.p_value.len = strlen((char*)entPhysicalDescrValue[oid_el2-1]);

		break;
    case entPhysicalVendorType:
    	//i have no registration for this mote id, hence, the default value would be {0 0}
    	entPhysicalVendorTypeValue[0] = 0;
    	entPhysicalVendorTypeValue[1] = 0;
    	object->varbind.value_type = BER_TYPE_OID;
    	object->varbind.value.p_value.ptr = entPhysicalVendorTypeValue;
    	object->varbind.value.p_value.len = 3;			//TODO: why 3 but not 2?
		break;
    case entPhysicalContainedIn:
    	object->varbind.value_type = BER_TYPE_INTEGER;
		object->varbind.value.i_value = 0;		//for debugging only
    	break;
    case entPhysicalClass:
    	object->varbind.value_type = BER_TYPE_INTEGER;
    	//supporting 6 entities: chassis, cpu, port, sensor x3 (temper + rssi + kwh meter)
    	object->varbind.value.i_value = entPhysicalClassValue[oid_el2-1];

    	break;

    case entPhysicalParentRelPos:
    case entPhysicalName:
    case entPhysicalHardwareRev:
    case entPhysicalFirmwareRev:
    case entPhysicalSoftwareRev:
    case entPhysicalSerialNum:
    case entPhysicalMfgName:
    case entPhysicalModelName:
    case entPhysicalAlias:
    case entPhysicalAssetID:
    case entPhysicalIsFRU:
    case entPhysicalMfgDate:
    case entPhysicalUris:
    default:
    	break;
    }
    return 0;
}

ptr_t* getNextPhysicalEntryOid(mib_object_t* object, u8t* oid, u8t len) {
    u32t oid_el1, oid_el2;
    u8t i;
    i = ber_decode_oid_item(oid, len, &oid_el1);
    i = ber_decode_oid_item(oid + i, len - i, &oid_el2);

    /* oid_el1 = [1..PHYSICALENTRYMAX]
     * oid_el2 = [1..PHYSICALNUMBER]
     */

    if (oid_el1 < PHYSICALENTRYMAX || (oid_el1 == PHYSICALENTRYMAX && oid_el2 < PHYSICALNUMBER)) {
        ptr_t* ret = oid_create();
        CHECK_PTR_U(ret);
        ret->len = 2;
        ret->ptr = malloc(2);
        CHECK_PTR_U(ret->ptr);
        if (oid_el2 < PHYSICALNUMBER) {
        	oid_el2++;
        } else if (oid_el2 >= PHYSICALNUMBER) {
        	if (oid_el1 < PHYSICALENTRYMAX) {
        		oid_el1++;
        		oid_el2 = 1;
        	} else {
        		return 0;
        	}
        }
        if (oid_el1 < 1) oid_el1 = 1;
        if (oid_el2 < 1) oid_el2 = 1;
        ret->ptr[0] = oid_el1;
        ret->ptr[1] = oid_el2;
        return ret;
    }
    return 0;
}
/*ENTITY-MIB initialization functions*/


/* -------- ENTITY-SENSOR_MIB initialization functions --------------*/
#define SENSORNUMBER 		3				//have 3 sensors: (temperature + rssi + kwh meter). See ENTITY-MIB

#define entPhySensorType	1
#define entPhySensorScale	2
#define entPhySensorPrecision	3
#define entPhySensorValue		4
#define entPhySensorOperStatus	5
#define entPhySensorUnitsDisplay	6
#define	entPhySensorValueTimeStamp	7
#define entPhySensorValueUpdateRate	8
#define SENSORENTRYMAX 				entPhySensorValueUpdateRate

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

s8t getEntityPhySensorEntry(mib_object_t* object, u8t* oid, u8t len) {
    u32t oid_el1, oid_el2;
    u8t i;
    //have 3 sensors: (temperature + rssi + kwh meter). See ENTITY-MIB
    s32t entPhySensorTypeValue[] PROGMEM = 		{SENSORDATATYPE_CELSIUS, SENSORDATATYPE_OTHER, SENSORDATATYPE_WATTS};
    s32t entPhySensorScaleValue[] PROGMEM = 		{SENSORDATASCALE_UNIT, SENSORDATASCALE_UNIT, SENSORDATASCALE_UNIT};
    s32t entPhySensorPrecisionValue[] PROGMEM = 	{0, 0, 0};		//value range from [-8, 9]
    s32t entPhySensorOperStatusValue[] PROGMEM = {SENSORSTATUS_OK, SENSORSTATUS_OK, SENSORSTATUS_OK};
    char *entPhySensorUnitsDisplayValue[] PROGMEM = 	{"Cel", "dB", "W"};
    int entPhySensorValueTimeStampValue[] PROGMEM =		{1234, 1234, 1234};
    unsigned int entPhySensorValueUpdateRateValue[] PROGMEM = 	{10000, 10000, 10000};			//in milliseconds

    i = ber_decode_oid_item(oid, len, &oid_el1);
    i = ber_decode_oid_item(oid + i, len - i, &oid_el2);

    if (len != 2)
        return -1;

    if (!(0 < oid_el2 && oid_el2 <= SENSORNUMBER))
    	return -1;

    switch (oid_el1) {
    case entPhySensorType:
    	object->varbind.value_type = BER_TYPE_INTEGER;
		object->varbind.value.i_value = entPhySensorTypeValue[oid_el2-1];
    	break;

    case entPhySensorScale:
    	object->varbind.value_type = BER_TYPE_INTEGER;
		object->varbind.value.i_value = entPhySensorScaleValue[oid_el2-1];
    	break;

    case entPhySensorPrecision:
    	object->varbind.value_type = BER_TYPE_INTEGER;
		object->varbind.value.i_value = entPhySensorPrecisionValue[oid_el2-1];
    	break;
    case entPhySensorValue:
    	//TODO: update sensor value from sensor
    	//here, i set a static value, for debugging purpose only
    	object->varbind.value_type = BER_TYPE_INTEGER;
		object->varbind.value.i_value = oid_el2;
    	break;
    case entPhySensorOperStatus:
    	//TODO: update sensor status from current condition of sensor
    	object->varbind.value_type = BER_TYPE_INTEGER;
		object->varbind.value.i_value = entPhySensorOperStatusValue[oid_el2-1];
    	break;
    case entPhySensorUnitsDisplay:
    	object->varbind.value_type = BER_TYPE_OCTET_STRING;
		object->varbind.value.p_value.ptr = entPhySensorUnitsDisplayValue[oid_el2-1];
		object->varbind.value.p_value.len = strlen(entPhySensorUnitsDisplayValue[oid_el2-1]);
    	break;
    case entPhySensorValueTimeStamp:
    	object->varbind.value_type = BER_TYPE_TIME_TICKS;
    	object->varbind.value.u_value = entPhySensorValueTimeStampValue[oid_el2-1];
    	break;
    case entPhySensorValueUpdateRate:
    	object->varbind.value_type = BER_TYPE_GAUGE;
		object->varbind.value.u_value = entPhySensorValueUpdateRateValue[oid_el2-1];
    	break;
    }
    return 0;
}

ptr_t* getNextPhySensorEntryOid(mib_object_t* object, u8t* oid, u8t len) {
    u32t oid_el1, oid_el2;
    u8t i;
    i = ber_decode_oid_item(oid, len, &oid_el1);
    i = ber_decode_oid_item(oid + i, len - i, &oid_el2);

    /* oid_el1 = [1..SENSORENTRYMAX]
     * oid_el2 = [1..SENSORNUMBER]
     */

    if (oid_el1 < SENSORENTRYMAX || (oid_el1 == SENSORENTRYMAX && oid_el2 < SENSORNUMBER)) {
        ptr_t* ret = oid_create();
        CHECK_PTR_U(ret);
        ret->len = 2;
        ret->ptr = malloc(2);
        CHECK_PTR_U(ret->ptr);
        if (oid_el2 < SENSORNUMBER) {
        	oid_el2++;
        } else if (oid_el2 >= SENSORNUMBER) {
        	if (oid_el1 < SENSORENTRYMAX) {
        		oid_el1++;
        		oid_el2 = 1;
        	} else {
        		return 0;
        	}
        }
        if (oid_el1 < 1) oid_el1 = 1;
        if (oid_el2 < 1) oid_el2 = 1;
        ret->ptr[0] = oid_el1;
        ret->ptr[1] = oid_el2;
        return ret;
    }
    return 0;
}
/*ENTITY-SENSOR-MIB initialization functions*/





/*
 * Initialize the MIB.
 */
s8t mib_init()
{
	/*
	 * SNMPv2-MIB
	 */

    if (add_scalar(&oid_system_desc, 0, BER_TYPE_OCTET_STRING, 0, &getSysDescr, &setSysDescr) == -1 ||
        add_scalar(&oid_system_time, 0, BER_TYPE_TIME_TICKS, 0, &getTimeTicks, 0) == -1  ||
        add_scalar(&oid_system_system, 0,BER_TYPE_OCTET_STRING, "snmp, modi by nqd", 0, 0) == -1) {
        	return -1;
    }

	/*
	 * if-mib
	 */
    if (add_scalar(&oid_if_number, 0, BER_TYPE_INTEGER, 0, &getIfNumber, 0) == -1) {
    	return -1;
    }
    if (add_table(&oid_if_table, &getIf, &getNextIfOid, 0) == -1) {
    	return -1;
    }

	/*
	 * ip-mib
	 * not available now
	 */

	/*
	 * ENTITY-MIB
	 */
    if (add_table(&oid_entPhysicalEntry, &getEntityPhysicalEntry, &getNextPhysicalEntryOid, 0) == -1) {
    	return -1;
    }
	/*
	 * ENTITY-SENSOR_MIB
	 */

    if (add_table(&oid_entPhySensorEntry, &getEntityPhySensorEntry, &getNextPhySensorEntryOid, 0) == -1) {
    	return -1;
    }
    return 0;
}
