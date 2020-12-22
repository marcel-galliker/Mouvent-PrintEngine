#include "comm.h"
#include "error.h"

#include <string.h>
#include <stdbool.h>

// Defines
#define STX	0x02
#define ETX 0x03

// Data Structures
typedef struct
{
	UINT32 commBufLen;
	UINT32 bufOk;
	UINT8 commBuf[COMM_BUF_SIZE];
	UINT8 crc;
	UINT8 data;
} SComm;

static SComm _Comm[COMM_CNT];

INT32 comm_init(UINT32 commNo)
{
	if(commNo >= COMM_CNT) return REPLY_ERROR;

	memset(&_Comm[commNo], 0, sizeof(_Comm[commNo]));

	return REPLY_OK;
}

INT32 comm_encode(void *src, UINT32 srcSize, UINT8 *dest, INT32 destSize, UINT32 *bytesWritten)
{
	UINT8 *in;
	UINT8 crc=0;
	UINT32 len=0;
	UINT32 pos=0;

	if (destSize<2*srcSize + 3 + 2) return REPLY_ERROR;

	dest[len++]=STX;
	for (in=src; srcSize; srcSize--, in++)
	{
		crc += ((*in)+(++pos));
		dest[len++] = 'A' + (((*in)&0xf0)>>4);
		dest[len++] = 'a' + ((*in)&0x0f);
	}
	dest[len++] = 'A' + ((crc&0xf0)>>4);
	dest[len++] = 'a' + (crc&0x0f);
	dest[len++] = ETX;

	*bytesWritten = len;
	return REPLY_OK;
}

INT32 comm_received(UINT32 commNo, UINT8 ch)
{
	SComm *comm = &_Comm[commNo];
	if (ch==STX)
	{
		memset(comm, 0, sizeof(SComm));
		return REPLY_ERROR;
	}

	if (ch==ETX)
	{
		if (comm->commBufLen>0)
		{
			if (comm->data==comm->crc)
			{
				comm->commBufLen--;
				comm->bufOk = true;
				return REPLY_OK;
			}
			else
			{
				memset(comm, 0, sizeof(SComm));
				return REPLY_ERROR;
			}
		}
		return REPLY_ERROR;
	}

	if (comm->commBufLen>=COMM_BUF_SIZE)
	{
		comm_init(commNo);
		return REPLY_ERROR;
	}

	if (ch>='A' && ch<'Z')
	{
		if (comm->commBufLen) comm->crc += (comm->data+comm->commBufLen);
		comm->data=((ch-'A')&0x0f)<<4;
	}
	else if (ch>='a' && ch<'z')
	{
		comm->data |= ((ch-'a') & 0x0f);
		comm->commBuf[comm->commBufLen++] = comm->data;
	}
	return REPLY_ERROR;
}

INT32 comm_get_data(UINT32 commNo, UINT8 *dest, UINT32 destSize, UINT32 *bytesReceived)
{
	SComm *comm = &_Comm[commNo];
	if (!comm->bufOk)  return REPLY_ERROR;
	if (destSize<comm->commBufLen) return REPLY_ERROR;
	memcpy(dest, comm->commBuf, comm->commBufLen);
	(*bytesReceived) = comm->commBufLen;
	return REPLY_OK;
}
