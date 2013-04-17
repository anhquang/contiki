#include "snmpd.h"
#include "collectd.h"

AUTOSTART_PROCESSES(&collectd_process, &snmpd_process);
/*---------------------------------------------------------------------------*/
