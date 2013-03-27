/*
 * Nguyen Quoc Dinh nqdinh@hui.edu.vn
 * Mar 2013
 */


#include <stdlib.h>


#include "contiki-lib.h"
#include "contiki-net.h"

#include "ntp.h"
#include "time.h"

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

static struct time_spec clock_offset;			//offset = 'real updated time' - clock time

void clock_get(struct time_spec *ts) {
	rtimer_clock_t tarcounter;
	clock_time_t tmp_scount;
	do {
	//	ts->sec = boottime + clock_seconds();
		ts->sec = clock_seconds();
		do {
			tarcounter = clock_counter();
			tmp_scount = clock_time();
		} while  (tarcounter != clock_counter());
		ts->nsec = tmp_scount * (1000000000 / CLOCK_SECOND)+tarcounter*(1000000000/(CLOCK_SECOND*256));
	}
	//while (ts->sec != (boottime+clock_seconds()));
	while (ts->sec != (clock_seconds()));
}

void clock_set_time(unsigned long sec,unsigned long nsec) {
	struct time_spec ori_time;

	clock_get(&ori_time);
	clock_offset.sec = sec - ori_time.sec;
	clock_offset.nsec = nsec - ori_time.nsec;
}
//****//
void clock_get_time(struct time_spec *ts) {
	clock_get(ts);
	//return with offset
	//CAUTION: this is with error some cases
	ts->sec += clock_offset.sec;
	ts->nsec += clock_offset.nsec;
}

/*
 * with ADJUST_THRESHOLD = 0, this function would never be touch
 * CAUTION: this function change timer register of cpu. Be aware that the timer register
 * in changed could lead to misbehavior of tasks which rely on timer.
 */

void clock_adjust_time(struct time_spec *delta) {
	if (delta->sec == 0L) {
		if (delta->nsec == 0L){
			adjcompare=0;
			return;
		} else {
			adjcompare = -delta->nsec / (1000000000 / (CLOCK_SECOND*256));
		}
	} else {
		adjcompare=-delta->sec * (CLOCK_SECOND * 256)+-delta->nsec / (1000000000/(CLOCK_SECOND*256));
	}

	if (adjcompare > 0)
		clock_inc_dec(adjcompare--);
	else if (adjcompare < 0)
		clock_inc_dec(adjcompare++);
}
