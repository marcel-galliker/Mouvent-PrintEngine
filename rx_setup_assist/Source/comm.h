#ifndef COMM_H_
#define COMM_H_

#include <stdint.h>
#include <type_defs.h>
#include "sa_tcp_ip.h"


// Defines
#define COMM_NUMBER_TCP	(0)

#define COMM_CNT 		(1)
#define COMM_BUF_SIZE	(256 * 3)


INT32 comm_init(UINT32 commNo);
INT32 comm_encode(void *src, UINT32 srcSize, UINT8 *dest, INT32 destSize, UINT32 *bytesWritten);
INT32 comm_received(UINT32 commNo, UINT8 ch);
INT32 comm_get_data(UINT32 commNo, UINT8 *pbuf, UINT32 bufsize, UINT32 *pbuflen);

#endif /* COMM_H_ */