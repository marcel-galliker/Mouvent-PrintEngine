#ifndef SYSTICK_H_
#define SYSTICK_H_
#include <type_defs.h>

void systick_init(void);
UINT32 sys_now(void);

#endif /* SYSTICK_H_ */