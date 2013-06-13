#include <stdlib.h>
#include <string.h>
#include "dev/cc2420.h"
#include "dev/leds.h"
#include "dev/light-sensor.h"
#include "dev/battery-sensor.h"
#include "dev/sht11-sensor.h"
#include "man-sensor-sky.h"

static char sensors[MAX_SENSORS_BUF_SIZE];
static u8_t sensors_buf_len;

#define ADD(...) do {                                                   \
	sensors_buf_len += snprintf(&sensors[sensors_buf_len], sizeof(sensors) - sensors_buf_len, __VA_ARGS__);      \
} while(0)

/*---------------------------------------------------------------------------*/
u8_t collect_view_arch_read_sensors(char * sensors_str_output, u8_t max_strlen)
{
	sensors_buf_len = 0;
	SENSORS_ACTIVATE(light_sensor);
	SENSORS_ACTIVATE(battery_sensor);
	SENSORS_ACTIVATE(sht11_sensor);
	ADD("'bat1':%u,'bat2':%u,'lig1':%u,'lig2':%u,'tem':%u,'hum':%u",
			battery_sensor.value(0),
			sht11_sensor.value(SHT11_SENSOR_BATTERY_INDICATOR),
			light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC),
			light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR),
			sht11_sensor.value(SHT11_SENSOR_TEMP),
			sht11_sensor.value(SHT11_SENSOR_HUMIDITY));
	SENSORS_DEACTIVATE(light_sensor);
	SENSORS_DEACTIVATE(battery_sensor);
	SENSORS_DEACTIVATE(sht11_sensor);
	if (sensors_buf_len > max_strlen) {
		return -1;
	} else {
		memcpy(sensors_str_output, sensors, sensors_buf_len);
		sensors_str_output[sensors_buf_len] = 0;
		return sensors_buf_len;
	}
}
/*---------------------------------------------------------------------------*/
