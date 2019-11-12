// ****************************************************************************
//
//	DIGITAL PRINTING - EEprom.h
//
//	
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "rx_def.h"
#include "rx_error.h"
#include "rx_term.h"
#include "rx_head_ctrl.h"
#include "nios.h"
#include "EEprom.h"

//--- ---------------------------------------

typedef struct
{
	char revisionNo[3];
	char partNo[13];
	char serialNo[10];
	int  week;
	int	 year;
	char flexSide[2];
	char thermistor[2];
	char driveVoltage[4];
	char flowResistance[4];
	char dropMass[3][4];
	char jetStraightness[3];
	int badNozzleE[25];
	int badNozzleM[25];
	char volumeUniformity[5];
} SEEpromData;

static SEEpromData _Data[MAX_HEADS_BOARD];

//--- prototypes ---------------------------------------------------

static void _display_number(char *name, int size, BYTE *data);
static void _display_str(char *name, int size, BYTE *data);

//--- eeprom_init_data ------------------------------------------
void eeprom_init_data(int headNo, BYTE *eeprom, SHeadEEpromInfo *pInfo)
{
	SEEpromData *pdata;
	int badE=0, badM=0;
	int done=FALSE;
	int val;
	BYTE buf[EEPROM_DATA_SIZE];
	BYTE tag;
	BYTE *src;
	BYTE *end;
	char str[10];
	
	if (headNo<0 || headNo>=MAX_HEADS_BOARD) return;
	
	pdata = &_Data[headNo];
	memset(pdata, 0, sizeof(SEEpromData));
	memset(str, 0, sizeof(str));
	memcpy(buf, eeprom, EEPROM_DATA_SIZE);

	src=buf;
	end=src+EEPROM_DATA_SIZE;
	while(!done && src<end)
	{
		if (*src++=='/')
		{
			tag = *src++;
			switch(tag)
			{
			case 'q':	memcpy(pdata->revisionNo,		src,  2); src+= 2; break;
			case 'p':	memcpy(pdata->partNo,			src, 10); src+=10; break;
			case 's':	memcpy(pdata->serialNo,			src,  9); src+= 9; break;
			case 'd':	memset(str, 0, sizeof(str));
						memcpy(str, src,  4);	src+= 4;
						val=atoi(str);
						pdata->year=2000+val%100;
						pdata->week=val/100;
						break;
			case 'h':	memcpy(pdata->flexSide,			src,  1); src+= 1; break;
			case 't':	memcpy(pdata->thermistor,		src,  1); src+= 1; break;
			case 'v':	memcpy(pdata->driveVoltage,		src,  3); src+= 3; break;
			case 'f':	memcpy(pdata->flowResistance,	src,  3); src+= 3; break;
			case 'l':	memcpy(pdata->dropMass[2],		src,  3); src+= 3; break;
			case 'm':	memcpy(pdata->dropMass[1],		src,  3); src+= 3; break;
			case 'n':	memcpy(pdata->dropMass[0],		src,  3); src+= 3; break;
			case 'r':	memcpy(pdata->jetStraightness,  src,  2); src+= 2; break;
			case 'b':	memset(str, 0, sizeof(str));
						memcpy(str, src,  3); src+= 3; 
						sscanf(str, "%x", &val);
						pdata->badNozzleM[badM++] = 2048-val;
						break;
			case 'e':	memset(str, 0, sizeof(str));
						memcpy(str, src,  3); src+= 3; 
						sscanf(str, "%x", &val);
						pdata->badNozzleE[badE++] = val;
						break;
			case 'u':	memcpy(pdata->volumeUniformity, src,  4); src+= 4; break;
			case 'c':	done=TRUE; break;
			default:
				break;
			}
			
		}
	}

	memset(pInfo, 0, sizeof(*pInfo));
	
	if (!*pdata->revisionNo && !*pdata->serialNo)
	{
		return;
		/*
		strcpy(pdata->revisionNo, "00");
		strcpy(pdata->partNo, "NO-HEAD");
		strcpy(pdata->serialNo, "12345678-90");
		pdata->week=4;
		pdata->year=2017;
		strcpy(pdata->driveVoltage, "123");
		strcpy(pdata->flowResistance, "1");
		strcpy(pdata->jetStraightness, "2");
		strcpy(pdata->volumeUniformity, "3");
		strcpy(pdata->dropMass[0], "12");
		strcpy(pdata->dropMass[1], "34");
		strcpy(pdata->dropMass[2], "56");
		pdata->badNozzleM[0]=111; badM=1;
		pdata->badNozzleE[0]=222; badE=1;
		*/
	}
	
	//--- fill info ----------------------------
	int i, n;
	if (*pdata->serialNo) pInfo->serialNo = 100*atoi(pdata->serialNo) + atoi(&pdata->serialNo[7]);
	else				  pInfo->serialNo = INVALID_VALUE;
	pInfo->week				= (UINT8) pdata->week;
	pInfo->year				= (UINT8)(pdata->year - 2000);
	pInfo->flowResistance	= (UINT8) atoi(pdata->flowResistance);
	pInfo->straightness		= (UINT8) atoi(pdata->jetStraightness);
	pInfo->uniformity		= (INT16) atoi(pdata->volumeUniformity);
	pInfo->voltage			= (UINT16)atoi(pdata->driveVoltage);
	for (i = 0; i < 3; i++)		pInfo->dropSize[i]	= (UINT16)atoi(pdata->dropMass[i]);
	n=0;
	for (i = 0; i < badM && n < SIZEOF(pInfo->badJets); i++)	pInfo->badJets[n++]	= (UINT16)pdata->badNozzleM[i];	
	for (i = 0; i < badE && n < SIZEOF(pInfo->badJets); i++)	pInfo->badJets[n++]	= (UINT16)pdata->badNozzleE[i];			
}

//--- eeprom_display ---------------------------------
void eeprom_display(void)
{
	int i;
	int bad, anyBad;
	int no[MAX_HEADS_BOARD];
		
	for(i=0; i<MAX_HEADS_BOARD; i++) no[i]= RX_HBConfig.reverseHeadOrder? MAX_HEADS_BOARD-1-i:i;

	term_printf("\n");
	term_printf("--- EEPROM ---    "); for (i=0; i<4; i++) term_printf(" ----- %d ----  ", i); term_printf("\n");
	term_printf("Format:           "); for (i=0; i<4; i++) term_printf("%12s   ", _Data[no[i]].revisionNo); term_printf("\n");
	term_printf("Part No:          "); for (i=0; i<4; i++) term_printf("%12s   ", _Data[no[i]].partNo); term_printf("\n");
	term_printf("Serial No:        "); for (i=0; i<4; i++) term_printf("%12s   ", _Data[no[i]].serialNo); term_printf("\n");
	term_printf("Data Code:        "); for (i=0; i<4; i++) term_printf("    w%02d/%d   ", _Data[no[i]].week, _Data[no[i]].year); term_printf("\n");
	term_printf("Flex Side:        "); for (i=0; i<4; i++) term_printf("%12s   ", _Data[no[i]].flexSide); term_printf("\n");
	term_printf("Thermistor:       "); for (i=0; i<4; i++) term_printf("%12s   ", _Data[no[i]].thermistor); term_printf("\n");
	term_printf("Drive Voltage:    "); for (i=0; i<4; i++) term_printf("%12s   ", _Data[no[i]].driveVoltage); term_printf("\n");
	term_printf("Flow Resistance:  "); for (i=0; i<4; i++) term_printf("%12s   ", _Data[no[i]].flowResistance); term_printf("\n");
	term_printf("Drop Mass S:      "); for (i=0; i<4; i++) term_printf("%12s   ", _Data[no[i]].dropMass[0]); term_printf("\n");
	term_printf("Drop Mass M:      "); for (i=0; i<4; i++) term_printf("%12s   ", _Data[no[i]].dropMass[1]); term_printf("\n");
	term_printf("Drop Mass L:      "); for (i=0; i<4; i++) term_printf("%12s   ", _Data[no[i]].dropMass[2]); term_printf("\n");
	term_printf("Jet Straightness: "); for (i=0; i<4; i++) term_printf("%12s   ", _Data[no[i]].jetStraightness); term_printf("\n");
	term_printf("Volume Uniformity:");     for (i=0; i<4; i++) term_printf("%12s   ", _Data[no[i]].volumeUniformity); term_printf("\n");
	for (bad=0; bad<SIZEOF(_Data[no[i]].badNozzleM); bad++)
	{
		term_printf("Bad Nozzle M[%02d]: ", bad);
		anyBad = FALSE;
		for (i=0; i<4; i++) 
		{
			if (_Data[i].badNozzleM[bad]) {term_printf("%12d   ", _Data[no[i]].badNozzleM[bad]); anyBad = TRUE; } 
			else						  term_printf("%12s   ", "-");  						
		}
		term_printf("\n");
		if (!anyBad) break;
	}
	for (bad=0; bad<SIZEOF(_Data[no[i]].badNozzleE); bad++)
	{
		term_printf("Bad Nozzle E[%02d]: ", bad);
		anyBad = FALSE;
		for (i=0; i<4; i++) 
		{
			if (_Data[i].badNozzleE[bad]) {term_printf("%12d   ", _Data[no[i]].badNozzleE[bad]); anyBad = TRUE; } 
			else						  term_printf("%12s   ", "-");  						
		}
		term_printf("\n");
		if (!anyBad) break;
	}
	
//	term_printf("user: "); for (i=0; i<4; i++) term_printf("%12s   ", RX_NiosStat.user_eeprom[i]); term_printf("\n");
}

//--- _display_number --------------------------------------------
static void _display_number(char *name, int size, BYTE *data)
{
	UINT32 val=0;
	memcpy(&val, data, size);
	term_printf("%s: %d\n", name, val);	
}

//--- _display_str --------------------------------------------
static void _display_str(char *name, int size, BYTE *data)
{
	char str[32];
	memset(str, 0, sizeof(str));
	memcpy(str, data, size);
	term_printf("%s: %s\n", name, str);	
}
