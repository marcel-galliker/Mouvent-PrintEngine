
#ifndef __TIMER_H__
#define __TIMER_H__

#include "rx_ink_common.h"

int    timer_init(void);
void   timer_main(void);

UINT32 timer_getTick(void);
void   timer_sleep(UINT32 ms);

int heat;
// macros to set, clear and test bit
#define BOOL(x) (!(!(x)))
#define bitset(arg,posn) ((arg) | (1L << (posn)))
#define bitclr(arg,posn) ((arg) & ~(1L << (posn)))
#define bittst(arg,posn) BOOL((arg) & (1L << (posn)))
#endif /* __TIMER_H__ */
