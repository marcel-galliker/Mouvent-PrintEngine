/******************************************************************************/
/** \file comm.c
 **
 ** Communication between NIOS and Conditioner
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/
//
//	protocol: <STX> coded(data) .... coded(crc) <ETX>
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include "string.h"	// memset
#include "comm.h"

//--- defines -------------------------------------------------------


#define COMM_CNT		4



#define STX	0x02
#define ETX 0x03


typedef struct
{
	int	 commBufLen;
	int  bufOk;
	char commBuf[COMM_BUF_SIZE];
	char crc;
	char data;
} SComm;


//--- module globals ---------------------------------------
static SComm _Comm[COMM_CNT];

//--- prototypes -------------------------------------------

//--- comm_init ----------------------------------------------
void comm_init(int commNo)
{
	memset(&_Comm[commNo], 0, sizeof(_Comm[commNo]));
}

//--- comm_encode --------------------------------------------
int comm_encode(void *pin, int inlen, UCHAR *pbuf, int bufsize, int *pbuflen)
{
	char *in;
	char crc=0;
	int len=0;
	int pos=0;

	if (bufsize<2*inlen + 3 + 2) return FALSE;

	pbuf[len++]=STX;
	for (in=pin; inlen; inlen--, in++)
	{
		crc += ((*in)+(++pos));
		pbuf[len++] = 'A' + (((*in)&0xf0)>>4);
		pbuf[len++] = 'a' + ((*in)&0x0f);
	}
	pbuf[len++] = 'A' + ((crc&0xf0)>>4);
	pbuf[len++] = 'a' + (crc&0x0f);
	pbuf[len++] = ETX;

	*pbuflen = len;
	return TRUE;
}

//--- comm_received --------------------------------------------
int  comm_received(int commNo, char ch)
{
	SComm *comm = &_Comm[commNo];
	if (ch==STX)
	{
		memset(comm, 0, sizeof(SComm));
		return FALSE;
	}

	if (ch==ETX)
	{
		if (comm->commBufLen>0)
		{
			if (comm->data==comm->crc)
			{
				comm->commBufLen--; // crc was counted!
				comm->bufOk = TRUE;
				//	arm_ptr->stat.cond[commNo].error.uart_read_error=0;
				return TRUE;
			}
			else
			{
				//	printf("Checksum Error\n");
				//	arm_ptr->stat.cond[commNo].error.uart_read_error+=1;
				//	arm_ptr->stat.cond[commNo].error.checksum_error+=1;
				memset(comm, 0, sizeof(SComm));
				return FALSE;
			}
		}
		return FALSE;
	}

	if (comm->commBufLen>=COMM_BUF_SIZE)
	{
		//printf("buffer overflow\n");
		comm_init(commNo);
		//	arm_ptr->stat.cond[commNo].error.uart_read_error+=1;
		//	arm_ptr->stat.cond[commNo].error.buffer_overflow+=1;
		return FALSE;
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
	return FALSE;
}

//--- comm_get_data -----------------------------------
int	 comm_get_data(int commNo, UCHAR *pbuf, int bufsize, int *pbuflen)
{
	SComm *comm = &_Comm[commNo];
	if (!comm->bufOk)  return FALSE;
	if (bufsize<comm->commBufLen) return FALSE;
	memcpy(pbuf, comm->commBuf, comm->commBufLen);
	(*pbuflen) = comm->commBufLen;
	return TRUE;
}
