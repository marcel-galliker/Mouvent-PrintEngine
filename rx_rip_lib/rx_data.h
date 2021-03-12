// ****************************************************************************
//
//	
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "export.h"

#define DATA_FIELD_CNT	256
#define FIELD_NAME_LEN	32

typedef enum // : INT32
{
	FF_Undef	= 0,
	FF_Fixed	= 1,
	FF_Variable = 2
} FileFormatEnum;

typedef struct 
{
    char	 name[FIELD_NAME_LEN];
    INT32    pos;
    INT32    len;
} SFieldDef;

typedef struct
{
    INT32    size;

	INT32	 id;

	char	 dataFile[256];
    INT32    header;
    INT32    unicode;
    INT32    codePage;
	INT32    recordCnt;
	INT32 nbRows;
	INT32 nbCols;


	FileFormatEnum			fileFormat;
    INT32					recLen;
	INT32					filter;
	char					fieldSep;

	SFieldDef    field[DATA_FIELD_CNT];
} SFileDef;

int dat_interprete_data_fields(UTF16 *text, UTF16 *src, int size);


#ifdef __cplusplus
	extern "C"{
#endif

EXPORT int  dat_load_file_def(void *doc, char *tempPath, SFileDef *pFileDef);
EXPORT int  dat_save_file_def(void *doc, char *tempPath, SFileDef *pFileDef,  char *dstPath);

EXPORT int dat_set_file_def	(SFileDef *pFileDef);
EXPORT int  dat_seek			(int recNo);
EXPORT int  dat_read_next_record (void);
EXPORT int  dat_get_field	(int no, BYTE *data, int maxsize);
EXPORT int  dat_get_buffer	(int pos, int len, BYTE *data);
EXPORT int  dat_set_buffer	(int pos, int len, BYTE *data);
EXPORT int  dat_analyse_file(SFileDef *pFileDef);
EXPORT int  dat_max_record_size(); 

#ifdef __cplusplus
	}
#endif