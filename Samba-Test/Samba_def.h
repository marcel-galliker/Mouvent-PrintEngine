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

//--- function reply values -----------------------------------
#define REPLY_OK	0
#define REPLY_ERROR 1

//--- macros -------------------------------------------------

#define SIZEOF(x) (sizeof(x)/sizeof(*(x)))

//--- nozzle alignment --------------------------------------
#define NOZZLE_CNT		2048
#define NOZZLE_OVERLAP	128
#define NOZZLE_MAX_Y	610
#define HEAD_UNITS		32

void Nozzles_init();


typedef struct
{
	int		byteOffset;
	UINT8	bitSet;
	int		delay;
} SLineMapping;

extern int			LineCnt;
extern SLineMapping	*LineMapping;
extern SLineMapping	_LineMapping[NOZZLE_CNT + NOZZLE_OVERLAP];
extern int			MaxY;

void Nozzles_initLineMapping(int backwards, int rightToLeft);
void Nozzles_initLineMapping_V2(int backwards, int rightToLeft);

typedef struct
{
	int		x;
	int		y;	
} SReverseLineMapping;

void Nozzles_initLineMapping_reverse();
void Nozzles_initLineMapping_reverse_V2();
extern SReverseLineMapping		ReverseLineMapping[NOZZLE_CNT];

typedef struct
{
	int width;
	int	height;
	int bitsPerPixel;
	int lineLen;	// in bytes
	int dataSize;
	int inverse;	
} SBmpInfo;


