// ****************************************************************************
//
//	matrix.c
//
//	main controller tasks.
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "rx_common.h"
#ifdef WIN32
	#include "io.h"
	#include "matrix32.h"
#else
	#include "mxapi.h"
	#include <time.h>
#endif
#include "rx_crc.h"
#include "rx_def.h"
#include "rx_error.h"
#include "matrix.h"

#define USER_CODE	4330780

short DNG_Port = 85;	// USB
short DNG_Count;
short DNG_Mem;
short DNG_MaxVar;

long  API_Version;     /* Versionsnummer der Matrix-API        */
long  DNG_Version;     /* Dongle-Versionsnummer                */


static short _Doungle = 0;

int mx_generate_code(UINT16 machineId, UINT16 inkId, UINT16 licNo, UINT32 volume, char *codeStr, int size);
int mx_generate_file(UINT16 machineId, UINT16 inkId, UINT16 licNo, UINT32 volume, char *dirpath);
int mx_add_license	(char *codeStr);
int mx_lic_encode	(SInkLicense *plic, char *codeStr, int size);
int mx_lic_decode	(UINT64 code[2], SInkLicense *plic);

//--- prototypes ---------------------------------------------------------------
static int  _encode_str_128(const UCHAR data[16], char *code, int codesize);
static void _decode_str_128(const char *code, UCHAR data[16]);

//--- allocation of doungle variables: -----------------------------------------
//	Var 0001:	machine ID
//	Var 0002:	LO_WORD: inkID[0]	HI_WORD: licNo[0]
//	Var 0003:	volume remaining[0]
//	Var 0004:
//
//	Var 0079:
//	END----------------------------

#pragma pack(1)
typedef struct
{
	UINT16	inkID;
	UINT16	licNo;
	UINT32	volume;	// [ml]
} SMatrixInkData;

typedef struct
{
	UINT32 machineID;				// Var 0001
	SMatrixInkData inkData[16];		// Var 0002 - 0033
} SMatrixData;

#pragma pack()

//--- mx_test -------------------------------------------------
void mx_test(void)
{
	int ret, i;
	short dongle;
	short port=1;
	UINT32 data[10];

	ret = Init_MatrixAPI();
	API_Version = GetVersionAPI();
	printf("API_Version: %d.%d\n", (int)(API_Version>>16), (int)(API_Version&0xffff));
	dongle = Dongle_Find();
	printf("Dongle_Find=%d\n", dongle);
	printf("Dongle_Count=%d\n", Dongle_Count(dongle));
	printf("MemSize=%d\n", Dongle_MemSize(0, dongle));
	printf("Model=%d\n", Dongle_Model(0, dongle));
	ret=Dongle_Version(0, dongle);
	printf("Version=%d.%d\n", ret>>16, ret&0xffff);
	printf("SerNr=%d\n", Dongle_ReadSerNr(USER_CODE, 0, dongle));
	printf("KeyFlag=%d\n", Dongle_GetKeyFlag(USER_CODE, 0, dongle));

	Dongle_ReadDataEx(USER_CODE, data, 1, 10, 0, dongle);
	for (i=0; i<10; i++)
	{
		printf("Data[%d]=0x%08x\n", i, data[i]);
	}

	char codeStr[128];
	mx_generate_code(1,2,3,1000, codeStr, sizeof(codeStr));
	mx_add_license(codeStr);
}


//--- mx_generate_code --------------------------------------------------------
int mx_generate_code(UINT16 machineId, UINT16 inkId, UINT16 licNo, UINT32 volume, char *codeStr, int size)
{
	SInkLicense	lic;

	lic.machineID	= machineId;
	lic.inkID		= inkId;
	lic.licNo		= licNo;
	lic.volume		= volume;
	#ifdef linux
	{
		struct timespec tp;
		clock_gettime(0, &tp);
		lic.code_1 = tp.tv_nsec;
	}
	#else
		lic.code_1	= (USHORT)GetTickCount();
	#endif
	lic.code_2		= lic.code_1 ^ 0xabcd;
	lic.crc16       = 0;
	lic.crc16	    = rx_crc16(&lic, sizeof(SInkLicense));	

	return mx_lic_encode(&lic, codeStr, size);	 
}

//--- mx_generate_file ----------------------------------------------
int mx_generate_file(UINT16 machineId, UINT16 inkId, UINT16 licNo, UINT32 volume, char *dirpath)
{
	char path[MAX_PATH];
	char codeStr[128];
	FILE *file;

	sprintf(path, "%s/Ink-m%04d-i%04d-%04d.lic", dirpath, machineId, inkId, licNo);

	file = fopen(path, "w");
	if (file)
	{
		mx_generate_code(machineId, inkId, licNo, volume, codeStr, sizeof(codeStr));
		fwrite(codeStr, 1, strlen(codeStr), file);
		fclose(file);
		return REPLY_OK;
	}
	return REPLY_ERROR;
}

//--- mx_lic_encode -------------------------------------------------
int mx_lic_encode(SInkLicense *plic, char *codeStr, int size)
{	
	int		i, ret;
	UINT64  code[2];

	memcpy(code, plic, 16);
	if (!_Doungle) _Doungle = Dongle_Find();
	for (i=0; i<2; i++)
	{
		if (ret=Dongle_EncryptData(USER_CODE, (INT32*)&code[i], 0, _Doungle)<0) return ret;
	}

	_encode_str_128((UCHAR*)code, codeStr, size);
	return REPLY_OK;
}

//--- mx_lic_decode -------------------------------------------------
int mx_lic_decode(UINT64 code[2], SInkLicense *plic)
{
	int i, ret;
	UINT16 crc;
	if (!_Doungle) _Doungle = Dongle_Find();
	for (i=0; i<2; i++)
	{
		if (ret=Dongle_DecryptData(USER_CODE, (INT32*)&code[i], 0, _Doungle)<0) return ret;
	}
	memcpy(plic, code, 16);
	crc = plic->crc16;
	plic->crc16 = 0;
	if (rx_crc16(plic, sizeof(SInkLicense))==crc) return REPLY_OK;
	else                                          return REPLY_ERROR;
}

//--- mx_add_license ----------------------------------------------------
int mx_add_license(char *codeStr)
{
	UINT64 code[2];
	SInkLicense	lic;
	SMatrixData data;
	int			i;

	_decode_str_128(codeStr, (UCHAR*)code);

	if (!_Doungle) _Doungle = Dongle_Find();

	if (mx_lic_decode(code, &lic)!=REPLY_OK) 
	{		
		_Doungle = 0;
		return Error(ERR_CONT, 0, "License invalid.");
	}

	Dongle_ReadDataEx(USER_CODE, (INT32*)&data, 1, (sizeof(data)+3)/4, 0, _Doungle);
	
	if (lic.machineID!=data.machineID) return Error(ERR_CONT, 0, "License dot for this machine (machineID=%d, licID=%d)", data.machineID, lic.machineID);
	
	for (i=0; i<SIZEOF(data.inkData); i++)
	{
		if (lic.inkID==data.inkData[i].inkID)
		{
			if (lic.licNo == data.inkData[i].licNo+1)
			{
				// found!
				data.inkData[i].volume += lic.volume;
				data.inkData[i].licNo  = lic.licNo;
				Dongle_WriteDataEx(USER_CODE, (INT32*)&data.inkData[i], 2+2*i, (sizeof(data.inkData[i])+3)/4, 0, _Doungle);
				return REPLY_OK;
			}
			else return Error(ERR_CONT, 0, "Linecse out of sequence. (req=%d, lic=%d)", data.inkData[i].licNo+1, lic.licNo);
		}
	}
	return Error(ERR_CONT, 0, "IncID not found.  (inkID=%d)", lic.inkID);	
}


static const char _CodeStr[]="1GZRSY3PD4J29TLM0XAF5ENB8UVH7CK6";

//--- encode_str_128 ------------------------------------------
static int _encode_str_128(const UCHAR data[16], char *code, int codesize)
{
	UINT64 check;
	unsigned int m, l, i;
	unsigned int	a,b;
	UINT64 d;

	m = strlen(_CodeStr);
	l=0;
	for (i=0; i<2; i++)
	{
		if ((int)l>=codesize) return 1;
		check = 0xffffffffffffffff;
		memcpy(&d, &data[8*i], 8);
		while (check)
		{
			if (l%5==4) code[l++]='-';
			code[l++] = _CodeStr[d%m];
			check /= m;
			d /= m;
		}
	}
	if ((int)l+2>=codesize) return 1;
	a=b=0;
	for (i=0; i+2<l; i)
	{
		a += code[i++];
		b += code[i++];
	}
	code[l++] = _CodeStr[a%m];
	code[l++] = _CodeStr[b%m];
	code[l]=0;
	return 0;
}

//--- _decode_str_128 ----------------------------------------
static void _decode_str_128(const char *code, UCHAR data[16])
{
	unsigned int m, i, p;
	int				l;
	UINT64		 d, check;
	
	//--- check ---
	d	  = 0;
	i	  = 1;
	check = 1;
	m = strlen(_CodeStr);
	l=strlen(code)-2;
	while (l--)
	{
		if (l<0) return;
		if (code[l]=='-') l--;
		for (p=0; _CodeStr[p]!=code[l]; p++);
		d = d*m + p;
		if (check*m > check) check *= m;
		else
		{
			memcpy(&data[8*i], &d, 8);
			if (i==0) return;
			i--;
			d=0;
			check=1;
		}
	}
}
