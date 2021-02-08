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
#include "rx_trace.h"
#include "rx_head_ctrl.h"
#include "rx_crc.h"
#include "tcp_ip.h"
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

static int _Init=FALSE;
static SEEpromData _Data[MAX_HEADS_BOARD];

//--- prototypes ---------------------------------------------------

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
	char crc[2];
	unsigned short cc;

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
						pdata->badNozzleE[badE++] = 2048-val;
						break;
			case 'u':	memcpy(pdata->volumeUniformity, src,  2); src+= 2; break;
			case 'c':	done=TRUE; 						
						break;
			default:
				break;
			}
			
		}
	}

	memset(pInfo, 0, sizeof(*pInfo));
    
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
	
//	term_printf("user: "); for (i=0; i<4; i++) term_printf("%12s   ", RX_NiosStat.eeprom_mvt[i]); term_printf("\n");
}

//--- eeprom_display_user -------------------------------------
void eeprom_display_user(void)
{
	int i;
	char str[MAX_PATH];
	int len;

	len=sprintf(str, "--- EEPROM ---    "); for (i=0; i<4; i++) len+=sprintf(&str[len], " ----- %d ----  ", i); TrPrintf(TRUE, str);
	len=sprintf(str, "clusterNo:        "); for (i=0; i<4; i++) len+=sprintf(&str[len], "%12d   ", _NiosStat->eeprom_mvt[i].clusterNo); TrPrintf(TRUE,str);
	len=sprintf(str, "flowResistance:   "); for (i=0; i<4; i++) len+=sprintf(&str[len], "%12d   ", _NiosStat->eeprom_mvt[i].flowResistance); TrPrintf(TRUE,str);
	len=sprintf(str, "dropletsPrinted:  "); for (i=0; i<4; i++) len+=sprintf(&str[len], "%12lld   ", _NiosStat->eeprom_mvt[i].dropletsPrinted); TrPrintf(TRUE,str);
}

//--- eeprom_main ---------------------------------------------------------------
int  eeprom_main(int ticks, int menu)
{
	if (_NiosMem==NULL) return REPLY_OK;
	if (menu)
	{
		if (!_Init)
		{
			int size=sizeof(SHeadEEpromDensity);
			if (size%32) Error(ERR_CONT, 0, "sizeof(SHeadEEpromDensity) muist be a multiple of 32!");
		}

		for (int head=0; head<MAX_HEADS_BOARD; head++)
		{
			memcpy(&RX_HBStatus->head[head].eeprom_mvt,		&_NiosMem->stat.eeprom_mvt[head],	  sizeof(RX_HBStatus->head[head].eeprom_mvt));

			if (RX_HBStatus->head[head].eeprom.serialNo==INVALID_VALUE)	_NiosMem->cfg.eeprom_fuji_readCnt[head]++;
			if (!_Init) _NiosMem->cfg.eeprom_mvt_readCnt[head]++;
			
			SHeadEEpromDensity data;
			memcpy(&data, &_NiosMem->stat.eeprom_density[head], sizeof(data));
			UINT8	crc1=data.crc;
			UINT8   crc2=rx_crc8(&data, sizeof(SHeadEEpromDensity)-1);

			/*
			TrPrintfL(TRUE, "EEprom[%d]: crc.rom=%d, crc.calc=%d, writes=%d, res=%d", head, crc1, crc2, _NiosMem->stat.eeprom_density_writeCnt[head], _NiosMem->stat.eeprom_density_writeRes[head]);
			if (head==0)
			{
				UINT8 *d=(UINT8*)&data;
				for (int n=0; n<sizeof(SHeadEEpromDensity); n++)
					TrPrintf(TRUE, "Density[%d.%d]=0x%02x", head, n, d[n]);
			}
			*/

			if (crc1 == crc2)
			{
				memcpy(&RX_HBStatus->head[head].eeprom_density, &_NiosMem->stat.eeprom_density[head], sizeof(RX_HBStatus->head[head].eeprom_density));
			}
			else 
			{
				if ((_NiosMem->stat.eeprom_density[head].crc | _NiosMem->stat.eeprom_density[head].voltage)==0)
				{
					// in case we have old data
					RX_HBStatus->head[head].eeprom_density.voltage = _NiosMem->stat.eeprom_mvt[head]._voltage;
					memcpy(&RX_HBStatus->head[head].eeprom_density.densityValue, &_NiosMem->stat.eeprom_mvt[head]._densityValue, sizeof(RX_HBStatus->head[head].eeprom_density.densityValue));
					memcpy(&RX_HBStatus->head[head].eeprom_density.disabledJets, &_NiosMem->stat.eeprom_mvt[head]._disabledJets, sizeof(RX_HBStatus->head[head].eeprom_density.disabledJets));
				}
				else 
				{
					memset(&RX_HBStatus->head[head].eeprom_density, 0, sizeof(RX_HBStatus->head[head].eeprom_density));
				} 
				_NiosMem->cfg.eeprom_density_readCnt[head]++;
			}
		}
		_Init = TRUE;
	}
	return REPLY_OK;
}

void eeprom_read(void)
{
	for (int head=0; head<MAX_HEADS_BOARD; head++)
	{
		_NiosMem->cfg.eeprom_fuji_readCnt[head]++;
		_NiosMem->cfg.eeprom_mvt_readCnt[head]++;
		_NiosMem->cfg.eeprom_density_readCnt[head]++;
	}
}

//--- eeprom_set_clusterNo ------------------
void eeprom_set_clusterNo(INT32 clusterNo)
{
	if (_NiosMem==NULL) return;	

	for (int headNo=0; headNo<MAX_HEADS_BOARD; headNo++)
	{
		SHeadEEpromMvt cfg;
		memcpy(&cfg, &_NiosMem->stat.eeprom_mvt[headNo], sizeof(cfg));
		cfg.clusterNo = clusterNo;
		memcpy(&_NiosMem->cfg.eeprom_mvt[headNo], &cfg, sizeof(cfg));
		_NiosMem->cfg.eeprom_mvt_writeCnt[headNo]++;
	}
}

//--- eeprom_reset_droplets_printed --------------------------------------------
void eeprom_reset_droplets_printed(int headNo)
{
	if (headNo<0 || headNo>=MAX_HEADS_BOARD || _NiosMem==NULL) return;

	SHeadEEpromMvt *mvt = &_NiosMem->cfg.eeprom_mvt[headNo];
	mvt->dropletsPrinted = 0;
	mvt->dropletsPrintedCRC = rx_crc8(&mvt->dropletsPrinted, sizeof(mvt->dropletsPrinted));
}

//--- eeprom_set_flowResistance --------------------------------
void eeprom_set_flowResistance(int headNo, int value)
{
	if (headNo<0 || headNo>=MAX_HEADS_BOARD || _NiosMem==NULL) return;

	SHeadEEpromMvt cfg;
	memcpy(&cfg, &_NiosMem->stat.eeprom_mvt[headNo], sizeof(cfg));
	cfg.flowResistance = value;
	cfg.flowResistanceCRC = rx_crc8(&cfg.flowResistance, sizeof(cfg.flowResistance));
	memcpy(&_NiosMem->cfg.eeprom_mvt[headNo], &cfg, sizeof(cfg));
	_NiosMem->cfg.eeprom_mvt_writeCnt[headNo]++;
}

//--- eeprom_add_droplets_printed ---------------------------------------
void eeprom_add_droplets_printed(int headNo, UINT32 droplets, int time)
{
	static UINT64 _droplets[MAX_HEADS_BOARD];
	static int    _time[MAX_HEADS_BOARD];
	if (headNo<0 || headNo>=MAX_HEADS_BOARD || _NiosMem==NULL) return;	

	_droplets[headNo]+=(UINT64)droplets;
	
	if (_time[headNo]==0) _time[headNo]=time;
	if (time-_time[headNo]>60000)
	{
		if (_droplets[headNo]>1000000)
		{		
			SHeadEEpromMvt cfg;
			memcpy(&cfg, &_NiosMem->stat.eeprom_mvt[headNo], sizeof(cfg));
			cfg.dropletsPrinted += _droplets[headNo]/1000000;
			cfg.dropletsPrintedCRC = rx_crc8(&cfg.dropletsPrinted, sizeof(cfg.dropletsPrinted));
			memcpy(&_NiosMem->cfg.eeprom_mvt[headNo], &cfg, sizeof(cfg));
			_NiosMem->cfg.eeprom_mvt_writeCnt[headNo]++;
			_droplets[headNo] = 0;
		}
		_time[headNo] = time;
	}
}

//--- eeprom_set_rob_pos ------------------------------------
void eeprom_set_rob_pos(int headNo, int angle, int dist)
{
	if (headNo<0 || headNo>=MAX_HEADS_BOARD || _NiosMem==NULL) return;	

	SHeadEEpromMvt cfg;
	memcpy(&cfg, &_NiosMem->stat.eeprom_mvt[headNo], sizeof(cfg));
	if (angle) cfg.robot.angle=angle;
	if (dist)  cfg.robot.dist=dist;
	cfg.robot.crc = rx_crc8(&cfg.robot, sizeof(cfg.robot)-1);
	memcpy(&_NiosMem->cfg.eeprom_mvt[headNo], &cfg, sizeof(cfg));
	_NiosMem->cfg.eeprom_mvt_writeCnt[headNo]++;
}

//--- eeprom_set_density ---------------------------------------------
void eeprom_set_density(SDensityMsg *pmsg)
{
	if (_NiosMem==NULL) return;	

	int head = 	pmsg->head%MAX_HEADS_BOARD;

	/*
	{
		UINT8 *d=(UINT8*)&data;
		for (int n=0; n<sizeof(SHeadEEpromDensity); n++) d[n]=_NiosMem->cfg.eeprom_density_writeCnt[head]+n;
	}
	*/
	pmsg->data.crc = rx_crc8(&pmsg->data, sizeof(SHeadEEpromDensity)-1);
	/*
	{
		TrPrintfL(TRUE, "EEprom[%d].Write, CRC=%d, writes=%d",  head, data.crc, _NiosMem->cfg.eeprom_density_writeCnt[head]+1);
		UINT8 *d=(UINT8*)&data;
		for (int n=0; n<sizeof(SHeadEEpromDensity); n++)
			TrPrintf(TRUE, "Write[%d.%d]=0x%02x", head, n, d[n]);
	}
	*/
	memcpy(&_NiosMem->cfg.eeprom_density[head], &pmsg->data, sizeof(SHeadEEpromDensity));
	_NiosMem->cfg.eeprom_density_writeCnt[head]++;
}

/*
void eeprom_test(int head)
{
	SDensityMsg msg;
	msg.head=head;
	eeprom_set_density(&msg);
}
*/