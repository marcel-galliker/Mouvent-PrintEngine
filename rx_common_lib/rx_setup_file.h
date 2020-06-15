// ****************************************************************************
//
//	DIGITAL PRINTING - SetupFile.h
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once
#include "rx_common.h"

#ifdef __cplusplus
extern "C"{
#endif

HANDLE setup_create();
void   setup_destroy(HANDLE setup);

int setup_load(HANDLE hsetup, const char *filename);
int setup_save(HANDLE hsetup, const char *filename);
char *setup_to_str(HANDLE hsetup);

char	*setup_get_error_str(HANDLE hsetup);

typedef enum 
{
	READ,
	WRITE
} EN_setup_Action;

int  setup_chapter		 (HANDLE hsetup, const char *name, int no, EN_setup_Action action);
int  setup_chapter_next  (HANDLE hsetup, EN_setup_Action action, char *name, int size);
int  setup_chapter_add	 (HANDLE hsetup, char *name);
int  setup_chapter_delete(HANDLE hsetup);
void setup_text		(HANDLE hsetup, EN_setup_Action action,  char  *val, int size, const char* def);
void setup_str		(HANDLE hsetup, const char *name, EN_setup_Action action,  char  *val, int size, const char* def);
void setup_uchar	(HANDLE hsetup, const char *name, EN_setup_Action action, UCHAR  *val, UCHAR  def);
void setup_uchar_arr(HANDLE hsetup, const char *name, EN_setup_Action action, UCHAR  *val, int cnt, INT32 def);
// void setup_uint8	(HANDLE hsetup, const char *name, EN_setup_Action action, UINT8  *val, UINT8  def);
void setup_int16	(HANDLE hsetup, const char *name, EN_setup_Action action, INT16  *val, INT16  def);
void setup_uint16	(HANDLE hsetup, const char *name, EN_setup_Action action, UINT16 *val, UINT16 def);
void setup_int16_arr(HANDLE hsetup, const char *name, EN_setup_Action action, INT16  *val, int cnt, INT16 def);
void setup_int32	(HANDLE hsetup, const char *name, EN_setup_Action action, INT32  *val, INT32  def);
void setup_int32_arr(HANDLE hsetup, const char *name, EN_setup_Action action, INT32  *val, int cnt, INT32 def);
void setup_uint32	(HANDLE hsetup, const char *name, EN_setup_Action action, UINT32 *val, UINT32 def);
void setup_double	(HANDLE hsetup, const char *name, EN_setup_Action action, double *val, double def);
void setup_enum		(HANDLE hsetup, const char *name, EN_setup_Action action, INT32  *val, char *enumstr);
void setup_int64	(HANDLE hsetup, const char *name, EN_setup_Action action, INT64  *val, INT64  def);
void setup_uint64	(HANDLE hsetup, const char *name, EN_setup_Action action, UINT64 *val, UINT64 def);
void setup_double	(HANDLE hsetup, const char *name, EN_setup_Action action, double *val, double def);
void setup_mac_addr	(HANDLE hsetup, const char *name, EN_setup_Action action, INT64  *val, INT64  def);
void setup_binary	(HANDLE hsetup, const char *name, EN_setup_Action action, void   *val, int size, int *len);
	
void setup_str_next	(HANDLE hsetup, HANDLE *attribute, char *name, int namesize, char  *val, int valsize);

void setup_enum8(HANDLE hsetup, const char *name, EN_setup_Action action, UCHAR *val, enumToStr converter);


#ifdef __cplusplus
}
#endif