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
/* Defines                                                                   */
/*****************************************************************************/
#define work_flash_base_adr 0x200C0000

/*****************************************************************************/
/* Function prototype                                                        */
/*****************************************************************************/
extern void write_data_to_flash (void);
extern void read_data_from_flash(void);
extern void work_flash_test     (void);

#endif /* __work_flash_operation_h__ */
