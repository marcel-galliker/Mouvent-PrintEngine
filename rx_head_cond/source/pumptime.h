#ifndef PUMPTIME_H
#define PUMPTIME_H

#include "cond_def_head.h"

void    save_pumptime   (UINT32 pumptime, int write_flash);
UINT32  load_pumptime   (void);
void    reset_pumptime  (void);
    
#endif // PUMPTIME_H
