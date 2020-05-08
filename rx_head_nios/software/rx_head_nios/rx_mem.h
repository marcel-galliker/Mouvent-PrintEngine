/******************************************************************************/
/** rx_mem.h
 **
 **
 **
 **	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/

#ifndef RX_MEM_H_
#define RX_MEM_H_

void  mem_init(void *mem, int size);
int	  mem_available(void);
void *mem_alloc(int size);



#endif /* RX_MEM_H_ */
