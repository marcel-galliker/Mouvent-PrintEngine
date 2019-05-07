/******************************************************************************/
/** \file work_flash_operation.h
 ** 
 ** read and write to work flash section
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/
 
#ifndef __watchdog_h__
#define __watchdog_h__

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include "IOMux.h" 
#include "pdl.h"

/*****************************************************************************/
/* Function prototype                                                        */
/*****************************************************************************/
void InitSwWatchdog(void);
void ClearSwWatchdog(void);



#endif /* __watchdog_h__ */
