#ifndef DEBUG_PRINTF
#define DEBUG_PRINTF
#include "stdio.h"

#ifdef DEBUG
#define DBG_PRINTF(msg, ...) printf(msg, ##__VA_ARGS__)
#else
#define DBG_PRINTF(msg, ...)
#endif

#endif // DEBUG_PRINTF
