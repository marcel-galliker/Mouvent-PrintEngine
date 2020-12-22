/*
 * ethernet.h
 *
 * Created: 05/05/2020 10:26:51
 *  Author: Patrick
 */ 


#ifndef ETHERNET_H_
#define ETHERNET_H_

#include <type_defs.h>

void ethernet_init(void);
void ethernet_tick(void);
INT32 ethernet_send(void* buffer, UINT32 length);


#endif /* ETHERNET_H_ */