#ifndef __TIME_H__
#define __TIME_H__

#include <inttypes.h>

#include "contiki.h"
#include "sys/clock.h"

void clock_set_time(unsigned long sec,unsigned long nsec);
void clock_adjust_time(struct time_spec *delta);
void clock_get_time(struct time_spec *ts);
void clock_get(struct time_spec *ts);

#endif
