// ****************************************************************************
//
//	DIGITAL PRINTING - RxGlobals.c
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "rx_common.h"
#include "rx_file.h"
#include "rx_hash.h"
#include "rx_setup_file.h"
#include "rx_setup_ink.h"

//--- prototypes -----------------------------------------------------------

static void	_calc_maxFreq(SInkDefinition *pink);

//--- setup_ink ----------------------------------------------------
int setup_ink(const char *filepath, SInkDefinition *pink, EN_setup_Action  action)
{
	if (action==READ)
	{
		HANDLE  file;
		char	str[128];
		char	hash[64];
		char	name[64];
		int read, i;

		memset(pink, 0, sizeof(*pink));
		file = setup_create();
		setup_load(file, filepath);

		if (setup_chapter(file, "WaveForm", -1, action)!=REPLY_OK) return REPLY_ERROR;

		if (setup_chapter(file, "Ink", -1, action)!=REPLY_OK) return REPLY_ERROR;

		//split_path(filepath, NULL, pink->fileName, str);
		strcpy(str, filepath);
		str[strlen(str)-4]=0;
		strncpy(pink->fileName, &str[strlen(PATH_WAVE_FORM)], sizeof(pink->fileName));

		setup_str   (file, "Name",			action,  pink->name,		sizeof(pink->name),			"name");
		setup_str   (file, "Description",	action,  pink->description,	sizeof(pink->description),	"description");
		setup_str	(file, "ColorRGB",		action,  str, sizeof(str), "");
		sscanf(str, "#%8X", &pink->colorRGB);
		setup_int32(file, "ColorCode",		action, &pink->colorCode, 0);
		setup_int32(file, "Temp",			action, &pink->temp,	  0);
		setup_int32(file, "TempMax",		action, &pink->tempMax,	  0);
		setup_int32(file, "TempChiller",	action, &pink->tempChiller,	  0);
		setup_int32(file, "Meniscus",		action, &pink->meniscus, 150);
		setup_int32(file, "Viscosity",		action, &pink->viscosity, 0);
		setup_int32(file, "DropSpeed",		action, &pink->dropSpeed, 0);
		setup_int32_arr(file, "PrintingSpeed",  action, pink->printingSpeed, SIZEOF(pink->printingSpeed), 0);		
		
//		setup_int32(file, "CondPresOut",	action, &pink->condPresOut,  150);
		setup_int32_arr(file, "FlushTime",  action, pink->flushTime, SIZEOF(pink->flushTime), 0);		
		{
			UINT gl[4];
			// default values ---
			gl[0] =					    0x08;
			gl[1] = 0x01 |		  	    0x08;
			gl[2] = 0x01 |		 0x04 | 0x08;
			gl[3] = 0x01 | 0x02| 0x04 | 0x08;
			setup_str(file, "GreyLevel",	action, str,  sizeof(str), "");
			sscanf(str, "%d %d %d %d", &gl[0], &gl[1], &gl[2], &gl[3]);
			for (i=0; i<4; i++) pink->greyLevel[i]=gl[i];
		}

		setup_chapter(file, "..", -1, action);

		if (setup_chapter(file, "item", -1, action)!=REPLY_OK) return REPLY_ERROR;
		for (i=0; i<SIZEOF(pink->wf); i++)
		{
			setup_uint16(file, "Position",	action, &pink->wf[i].pos,	0);
			setup_uint16(file, "Voltage",	action, &pink->wf[i].volt,	0);
		//	printf("Voltage: %d:\t%d\n", pos, volt);
			setup_chapter_next(file, action, name, sizeof(name));
			if (strcmp(name,"item")) break;
		}
		setup_chapter(file, "..", -1, action);

		if (setup_chapter(file, "check", -1, action)!=REPLY_OK) return REPLY_ERROR;
		setup_text(file, action, str, sizeof(str), "default");

		setup_destroy(file);
		
		{ //--- chekcsum
			int	size = (int)rx_file_get_size(filepath);
			if (size)
			{
				char *buffer = (char*) malloc(size);
				char *pos;
				FILE *f;
				f = rx_fopen(filepath, "rb", _SH_DENYNO);
				read = (int)fread(buffer, 1, size, f);
				fclose(f);
				pos=strstr(buffer, "<check>");
				*hash=0;
				if (pos)
				{
					while (*pos!='>') pos--;
					rx_hash_mem_str(buffer, pos+1-buffer, hash);					
				}
				free(buffer);
			}
		}

		if (strcmp(str, hash))
		{
			memset(pink, 0, sizeof(*pink));
			return REPLY_ERROR;
		}

		_calc_maxFreq(pink);
	}
	return REPLY_OK;
}


//--- _calc_maxFreq -------------------------------------------
static void	_calc_maxFreq(SInkDefinition *pink)
{
	int trace=FALSE;
	int i, droplet, end;
	int start[MAX_GREY_LEVELS];
	double clock=2000/140.0; //12.5; // ns
	double time;

	memset(start, 0, sizeof(start));

	droplet = 0;
	for (i=0; i<SIZEOF(pink->wf); i++)
	{
		if(trace) printf("%03d: %03d  %03d", i, pink->wf[i].pos, pink->wf[i].volt);
		if (pink->wf[i].volt==0 && pink->wf[i+1].volt>0) 
		{
			if(trace) printf("   >>>");
			start[droplet++]=pink->wf[i].pos*140/160;
		}
		if (i>0 && pink->wf[i].volt==0 && pink->wf[i-1].volt>0) 
		{
			if(trace) printf("   <<<");
			end=pink->wf[i].pos*140/160;
		}
		if(trace) printf("\n");
	}
	while (droplet<MAX_GREY_LEVELS)
		start[droplet++] = end;
	if(trace)
	{
		printf("done\n");
		for (i=0; i<droplet; i++)
		{
			printf("length[%d]: %04d\n", i, start[i+1]-start[i]);
		}
		printf("total len=%d\n", end-start[0]);
	}
	int len;
	for (i=0; i<4; i++)
	{
//		len=WF_FIRST_PULSE_POS+WF_FILLER;
		len=0;
		for (droplet=0; droplet<MAX_GREY_LEVELS; droplet++)
		{
			if (pink->greyLevel[i] & (1<<droplet)) 
				len+=(start[droplet+1]-start[droplet]);
		}
		len  = len+WF_FILLER+20;
		time = (int)(len*clock);
		double khz=1000000/time;
        double mmin = khz*25.4/1200.0*60.0;
//        double PulseLength = time/1000;
 //       double PulseFreq   = khz;
 //       double PulseSpeed  = mmin;
//		pink->maxFreq[i] = (int)(1000*khz);
//		if(trace) printf("pulse length[%d] %d Hz, %d m/min\n", i, pink->maxFreq[i], (int)(mmin*1000));
		if (mmin>0) pink->maxSpeed[i] = (int)mmin;
		else	    pink->maxSpeed[i] = 0;
	}
}
