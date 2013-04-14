/* TODO:
 * 	save some setting to FLASH
 * 	timesticks
 * 	physical vendor type
 * CHANGE:
 * 	number of sensors
 */

#include "mib-init.h"
#include "ber.h"
#include "utils.h"
#include "logging.h"

#include "net/rime.h"
#include "mib-constant.h"

#include "cfs/cfs.h"

#if SIMULATION
#include "lib/random.h"
#endif

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
 *NOTE: I change iftable into table, not scala anymore.
 */
static u8t ber_oid_if_number[] PROGMEM    = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x02, 0x01, 0x00};
static ptr_t oid_if_number PROGMEM        = {ber_oid_if_number, 8};

static u8t ber_oid_if_table[] PROGMEM     = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x02, 0x02, 0x01};
static ptr_t oid_if_table PROGMEM         = {ber_oid_if_table, 8};

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

/* oid value variable */

static struct snmp_sysname_t sysname;

#if CONTIKI_TARGET_ZIGD
int snmp_fs_write(void *data, u8t len, u8t *filename) {
	int fd_write;
	fd_write = cfs_open(filename, CFS_WRITE);
	if(fd_write != -1) {
		cfs_write(fd_write, data, len);
		cfs_close(fd_write);
		return 0;
	}
	return -1;
}
int snmp_fs_read(void *data, u8t len, u8t *filename) {
	int fd_read;
	fd_read = cfs_open(filename, CFS_READ);
	if(fd_read != -1) {
		cfs_read(fd_read, data, len);
		cfs_close(fd_read);
		return 0;
	}
	return -1;
}
#endif

ptr_t* getNextOid(mib_object_t* object, u8t* oid, u8t len, u8t ENTRYMAX, u8t NUMBER)
{
    u32t oid_el1, oid_el2;
    u8t i;
    i = ber_decode_oid_item(oid, len, &oid_el1);
    i = ber_decode_oid_item(oid + i, len - i, &oid_el2);

    /* oid_el1 = [1..ENTRYMAX]
     * oid_el2 = [1..NUMBER]
     */

    if (oid_el1 < ENTRYMAX || (oid_el1 == ENTRYMAX && oid_el2 < NUMBER)) {
        ptr_t* ret = oid_create();
        CHECK_PTR_U(ret);
        ret->len = 2;
        ret->ptr = malloc(2);
        CHECK_PTR_U(ret->ptr);
        if (oid_el2 < NUMBER) {
        	oid_el2++;
        } else if (oid_el2 >= NUMBER) {
        	if (oid_el1 < ENTRYMAX) {
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

/**** SNMPv2-MIB initialization functions ****************/

#if CONTIKI_TARGET_AVR_RAVEN
extern unsigned long seconds;
#else
clock_time_t systemStartTime;
#endif
u32t SysUpTime()
{
    #if CONTIKI_TARGET_AVR_RAVEN
        return seconds * 100;
    #else
        return (clock_time() - systemStartTime)/ 10;
    #endif
}
//read only
s8t getSysUpTime(mib_object_t* object, u8t* oid, u8t len)
{
    object->varbind.value.u_value = SysUpTime();
    return 0;
}

//read & write
s8t getSysName(mib_object_t* object, u8t* oid, u8t len)
{
	if (!object->varbind.value.p_value.len) {
		object->varbind.value.p_value.ptr = sysname.sysname;
		object->varbind.value.p_value.len = sysname.sysnamelen;
	}
	return 0;
}
s8t setSysName(mib_object_t* object, u8t* oid, u8t len, varbind_value_t value)
{
	u8t i;
	if (!object->varbind.value.p_value.len < SYSNAME_LEN) {
		memset(sysname.sysname, 0, SYSNAME_LEN);
		for (i=0; i<value.p_value.len; i++){
			sysname.sysname[i] = value.p_value.ptr[i];
		}
		sysname.sysnamelen = value.p_value.len;

		//save to flash, for Vmote only, since sky run out of rom already
#if CONTIKI_TARGET_ZIGD
		snmp_fs_write(&sysname, sizeof(struct snmp_sysname_t), SYSNAME_FS_FILENAME);
#endif
	}
	return 0;
}

/**** IF-MIB initialization functions ****************/
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
			object->varbind.value.p_value.ptr = (u8t*)SNMP_IFDESCR;
			object->varbind.value.p_value.len = strlen(SNMP_IFDESCR);
			break;
        case ifType:
        	object->varbind.value_type = BER_TYPE_INTEGER;
        	object->varbind.value.i_value = SNMP_IANAIFTYPE_MIB_IEEE802154;
        	break;
        case ifMtu:
        	object->varbind.value_type = BER_TYPE_INTEGER;
        	object->varbind.value.i_value = SNMP_MTU_IEEE802154;
        	break;
        case ifSpeed:
        	object->varbind.value_type = BER_TYPE_GAUGE;
        	object->varbind.value.u_value = SNMP_SPEED_IEEE802154;
        	break;
        case ifPhysAddress:
        	object->varbind.value_type = BER_TYPE_OCTET_STRING;
			object->varbind.value.p_value.ptr = (u8t*)&rimeaddr_node_addr;
			object->varbind.value.p_value.len = sizeof(rimeaddr_t);
        	break;
        default:
            break;
    }
    return 0;
}

ptr_t* getNextIfOid(mib_object_t* object, u8t* oid, u8t len)
{
    return getNextOid(object, oid, len, IFENTRYMAX, IFNUMBER);
}
/*end IF-MIB initialization functions*/

/* -------- ENTITY-MIB initialization functions --------------*/
s8t getEntityPhysicalEntry(mib_object_t* object, u8t* oid, u8t len) {
    u32t oid_el1, oid_el2;
    u8t i;
    u8t entPhysicalVendorTypeValue[] PROGMEM = ENT_PHYSICAL_VENDOR_TYPE_VALUE;
    char entPhysicalClassValue[] PROGMEM = ENT_PHYSICAL_CLASS_VALUE;
    char *entPhysicalDescrValue[] PROGMEM = ENT_PHYSICAL_DESCR_VALUE;

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

ptr_t* getNextPhysicalEntryOid(mib_object_t* object, u8t* oid, u8t len)
{
	return getNextOid(object, oid, len, PHYSICALENTRYMAX, PHYSICALNUMBER);
}
/*ENTITY-MIB initialization functions*/


/* -------- ENTITY-SENSOR_MIB initialization functions --------------*/
s8t getEntityPhySensorEntry(mib_object_t* object, u8t* oid, u8t len)
{
    u32t oid_el1, oid_el2;
    u8t i;
    //have 3 sensors: (temperature + rssi + kwh meter). See ENTITY-MIB
    s32t entPhySensorTypeValue[] PROGMEM = 				ENT_PHYSENSOR_TYPE_VALUE;
    s32t entPhySensorScaleValue[] PROGMEM = 			ENT_PHYSENSOR_SCALE_VALUE;
    s32t entPhySensorPrecisionValue[] PROGMEM = 		ENT_PHYSENSOR_PRECISION_VALUE;
    s32t entPhySensorOperStatusValue[] PROGMEM = 		ENT_PHYSENSOR_OPER_STATUS_VALUE;
    char *entPhySensorUnitsDisplayValue[] PROGMEM = 	ENT_PHYSENSOR_UNITS_DISPLAY_VALUE;
    int entPhySensorValueTimeStampValue[] PROGMEM =		ENT_PHYSENSOR_VALUE_TIMESTAMP_VALUE;
    unsigned int entPhySensorValueUpdateRateValue[] PROGMEM = ENT_PHYSENSOR_VALUE_UPDATE_RATE_VALUE;			//in milliseconds

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
#if SIMULATION
   		object->varbind.value.i_value = random_rand()%10;
#else
   		object->varbind.value.i_value = oid_el2;
#endif
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

ptr_t* getNextPhySensorEntryOid(mib_object_t* object, u8t* oid, u8t len)
{
	return getNextOid(object, oid, len, SENSORENTRYMAX, SENSORNUMBER);
}
/*ENTITY-SENSOR-MIB initialization functions*/


/*
 * initialize oid value *
 */
void oid_val_init()
{
#if CONTIKI_TARGET_ZIGD
	struct snmp_sysname_t sysname_fs;

	if (snmp_fs_read(&sysname_fs, sizeof(struct snmp_sysname_t), SYSNAME_FS_FILENAME) != -1) {
		strcpy(sysname.sysname, sysname_fs.sysname);
		sysname.sysnamelen = ((sysname_fs.sysnamelen <= SYSNAME_LEN)? sysname_fs.sysnamelen : SYSNAME_LEN);
	} else {
#endif
	strcpy(sysname.sysname, (u8t*)SNMP_SYSNAME);
	sysname.sysnamelen = strlen(SNMP_SYSNAME);

#if CONTIKI_TARGET_VMOTE || CONTIKI_TARGET_ZIGD
	}
#endif
}

/*
 * Initialize the MIB.
 */
s8t mib_init()
{
	oid_val_init();

	/*
	 * SNMPv2-MIB
	 */

    if (add_scalar(&oid_system_desc, 0, BER_TYPE_OCTET_STRING, SNMP_SYSDESCR, 0, 0) == -1 ||
        add_scalar(&oid_system_time, 0, BER_TYPE_TIME_TICKS, 0, &getSysUpTime, 0) == -1  ||
        add_scalar(&oid_system_system, 0,BER_TYPE_OCTET_STRING, 0, &getSysName, &setSysName) == -1) {
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
