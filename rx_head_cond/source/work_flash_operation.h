/******************************************************************************/
/** \file work_flash_operation.h
 ** 
 ** read and write to work flash section
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/
 
#ifndef __work_flash_operation_h__
#define __work_flash_operation_h__

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include "IOMux.h" 
#include "pdl.h"

/*****************************************************************************/
/* Function prototype                                                        */
/*****************************************************************************/
extern void flash_write_data    (UINT32 pumptime);
extern UINT32 flash_read_data   (void);
extern void flash_test          (void);

#endif /* __work_flash_operation_h__ */
