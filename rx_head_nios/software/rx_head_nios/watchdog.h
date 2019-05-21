/*
 * watchdog.h
 *
 *  Created on: 31.08.2016
 *      Author: stefan
 */

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#define WATCHDOG_TIME_MS 5000  // milliseconds // maximal 10'737 ms
#define WATCHDOG_CHECK_FP FALSE

void watchdog_init(void);

void watchdog_toggle(int watchdog_time, int check_FP);
void watchdog_start_debug_mode(void);

#endif /* WATCHDOG_H_ */
