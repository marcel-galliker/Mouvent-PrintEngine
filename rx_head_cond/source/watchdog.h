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
/* Defines                                                                   */
/*****************************************************************************/
#define SWWDT_DELAY_S     5.0f

/*****************************************************************************/
/* Function prototype                                                        */
/*****************************************************************************/
void InitSwWatchdog(void);
void ClearSwWatchdog(void);

void watchdog_toggle24V(void);


#endif /* __watchdog_h__ */
