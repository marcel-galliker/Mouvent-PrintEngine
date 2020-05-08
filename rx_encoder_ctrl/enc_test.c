// ****************************************************************************
//
//	DIGITAL PRINTING - enc_test.c
//
//	head-control-client
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "rx_common.h"
#include "rx_fpga.h"
#include "rx_error.h"
#include "rx_term.h"
#include "fpga_def_enc.h"
#include "fpga_enc.h"
#include "enc_test.h"

#ifdef linux
	#include <sys/mman.h>
#endif

#ifdef DEBUG
	// #define TEST_FIREPULSE
	// #define TEST_SPEED
#endif

// #define TEST_ENCODER

#ifdef TEST_ENCODER
	extern SEncFpga		*Fpga;
	extern SEncFpgaCorr	*FpgaCorr;	
#endif
//-------------------------------------------------

static INT32 *_Val[4];
static int   _mem_pointer[4];
static int   _mem_pointer_in[4];
static INT32 _mem_cnt_in[4];
static INT32 _mem_cnt_out;
static int	_LastFlushTick=0;

static int	 _LogNo=0;
static FILE	*_LogFile[4] = {NULL, NULL, NULL, NULL};
	
//--- test_init -----------------------------------
void test_init(void)
{
// #ifdef DEBUG
	int memId;
	memId = open("/dev/mem", O_RDWR | O_SYNC);
	if (memId==-1) Error(ERR_CONT, 0, "Could not open memory handle.");

	_Val[0] = (INT32*)rx_fpga_map_page(memId, TEST_VALUE_ADDR_0, TEST_VALUE_CNT*sizeof(INT32), TEST_VALUE_CNT*sizeof(INT32));
	_Val[1] = (INT32*)rx_fpga_map_page(memId, TEST_VALUE_ADDR_1, TEST_VALUE_CNT*sizeof(INT32), TEST_VALUE_CNT*sizeof(INT32));
	_Val[2] = (INT32*)rx_fpga_map_page(memId, TEST_VALUE_ADDR_2, TEST_VALUE_CNT*sizeof(INT32), TEST_VALUE_CNT*sizeof(INT32));
	_Val[3] = (INT32*)rx_fpga_map_page(memId, TEST_VALUE_ADDR_3, TEST_VALUE_CNT*sizeof(INT32), TEST_VALUE_CNT*sizeof(INT32));
// #endif
}

//--- test_cfg_done --------------------
void test_cfg_done(void)
{
#ifdef TEST_ENCODER
	char str[MAX_PATH];
	int i, len, n;
		
	_LogNo++;
	_LastFlushTick = 0;

	for (n=0; n<SIZEOF(_mem_pointer); n++)
	{
		if (_LogFile[n]!=NULL) fclose(_LogFile[n]);
		_mem_pointer[n]=_mem_pointer_in[n]=Fpga->stat.mem_pointer[n];
		_mem_cnt_in[n]=0;
		_mem_cnt_out=0;
		/*
		sprintf(str, "%sencoder_%d_%d.csv", PATH_TEMP, _LogNo, n);
		_LogFile[n] = fopen(str, "w");
		if (_LogFile[n])
		{
			Error(WARN, 0, "Created Encoder Log >>%s<<", str);
		
			for (i=0; i<8; i++) fprintf(_LogFile[n], "Encoder[%d]\tCorrection=%d\tpar=%d\n", i, Fpga->cfg.encIn [i].correction, FpgaCorr->par[i].par);
		
			for (i=0; i<8; i++) fprintf(_LogFile[n], "Encoder[%d]\tStartPosFwd=%d\tStartPosBwd=%d\n", i, Fpga->cfg.pg[i].enc_start_pos_fwd, Fpga->cfg.pg[i].enc_start_pos_bwd);
		
			len=0;
			for (i=0; i<2; i++) len+=sprintf(&str[len], "InPos%d\t", i);
			for (i=0; i<8; i++) len+=sprintf(&str[len], "OutPos%d\t", i);
			for (i=0; i<8; i++) len+=sprintf(&str[len], "Speed%d\t", i);
			for (i=0; i<8; i++) len+=sprintf(&str[len], "PG%d\t", i);
			sprintf(&str[len], "\n");
			fprintf(_LogFile[n], str);
			fprintf(_LogFile[n], "val%d\n", n+1);
			fflush(_LogFile[n]);		
		}
		*/
	}
	n=0;
	sprintf(str, "%sencoder_%d_%d.csv", PATH_TEMP, _LogNo, n);
	_LogFile[n] = fopen(str, "w");
	if (_LogFile[n])
	{
		Error(WARN, 0, "Created Encoder Log >>%s<<", str);				
		len=0;
		for (i=0; i<2; i++) len+=sprintf(&str[len], "Pos%d\t", i);
		sprintf(&str[len], "\n");
		fprintf(_LogFile[n], str);
		fprintf(_LogFile[n], "val%d\n", n+1);
		fflush(_LogFile[n]);		
	}
#endif
}

//--- test_do ------------------------------------
void test_do(int ticks)
{
#ifdef TEST_FIREPULSE
	if (_Val[0])
	{
		int i;
		
		term_printf("\n");
		term_printf("FIREPULSE TEST\n");
		term_printf("%6s %6s %6s %6s\n", "#", "inc", "enc-FP", "hd-FP");
		for (i=0; i<VALUE_CNT; i+=1000)
		{
			term_printf("%6d %6d %6d %6d\n", i, _Val[0][i], _Val[1][i], _Val[2][i]);
		}
		i=VALUE_CNT-1;
		term_printf("%6d %6d %6d %6d\n", i, _Val[0][i], _Val[1][i], _Val[2][i]);
	}
#endif
	
#ifdef TEST_ENCODER
	int i, end, n;
	char str[32];
	
	if (_LogFile[0])
	{
		/*
		end=FALSE;
		for (n=0; !end&&n<2000; n++)
		{
			end=TRUE;
			for (i=0; i<SIZEOF(_mem_pointer); i++)
			{
				if (Fpga->stat.mem_pointer[i]!=_mem_pointer[i])
				{
					end=FALSE;
					if ((_mem_pointer[i]&0x7ff)==0x7ff) fprintf(_LogFile[i], "%6d\n", _Val[i][_mem_pointer[i]-1]);
					else								fprintf(_LogFile[i], "%6d\n", _Val[i][_mem_pointer[i]]);	
					if (++_mem_pointer[i]>=TEST_VALUE_CNT) _mem_pointer[i]=0;
				}
			}		
		}
		*/
		end=FALSE;
		for (n=0; !end&&n<2000; n++)
		{
			end=TRUE;
			for (i=0; i<2; i++)
			{
				if (Fpga->stat.mem_pointer[i]!=_mem_pointer_in[i])	
				{
					end=FALSE;
					_mem_cnt_in[i]++;				
					if (++_mem_pointer_in[i]>=TEST_VALUE_CNT) _mem_pointer_in[i]=0;
				}
			}
		}
		while(_mem_cnt_in[0]>_mem_cnt_out && _mem_cnt_in[1]>_mem_cnt_out)
		{
			fprintf(_LogFile[0], "%6d\t%06d\n", _Val[0][(_mem_pointer[0]+_mem_cnt_out)&0x1fff], _Val[1][(_mem_pointer[1]+_mem_cnt_out)&0x1fff]);	
			_mem_cnt_out++;
		}
	}
	if (ticks+1000>_LastFlushTick)
	{
		for (i=0; i<SIZEOF(_LogFile); i++)
		{
			if (_LogFile[i]) fflush(_LogFile[i]);							
		}
		_LastFlushTick=ticks;		
	}
#endif	
}

//--- test_write_csv ------------------------------------
void test_write_csv(char *filename)
{
	if (_Val[0])
	{
		int i;
		INT32 enc0;
		INT32 enc1;
		FILE *f;
		char path[MAX_PATH];
		sprintf(path, PATH_TEMP "%s", filename);
		f = fopen(path, "w");
		if (f!=NULL)
		{
			//fprintf(f, "%6s\t%6s\t%6s\t%6s\n", "#", "val0", "val1", "val2", "val3");
			//fprintf(f, "%6s\t%6s\t%6s\t%6s\t%6s\n", "#", "enc0_pos", "enc0_step", "enc1_pos", "enc1_step");
			fprintf(f, "%6s\t%6s\t%6s\n", "#", "substrokes", "inpos times 8");
			for (i = 1; i < TEST_VALUE_CNT - 1; i++) // for (i=1; i<29000-1; i++)
			{
				enc0 = _Val[0][i];
				enc1 = _Val[1][i];
				//fprintf(f, "%6d\t%6d\t%6d\t%6d\t%6d\n", i, _Val[0][i], _Val[1][i], _Val[2][i],  _Val[3][i]);
				//fprintf(f, "%6d\t%6d\t%6d\t%6d\t%6d\n", i, ((enc0) & 0xffff), ((enc0) >> 16), ((enc1) & 0xffff), ((enc1) >> 16));
				fprintf(f, "%6d\t%6d\t%6d\n", i, _Val[0][i], _Val[1][i]);
			}
			fclose(f);			
		}
	}
}

//--- test_write_csv_encoder ------------------------------------
void test_write_csv_new(void)
{
#ifdef DEBUG
	if (_Val[0])
	{
		int i;
		int diff;
		int base;
		int e1, e2;
		FILE *f;
		f = fopen(PATH_TEMP "encoder_test.csv", "w");
		if (f!=NULL)
		{
			fprintf(f, "%6s %6s %6s %6s\n", "#", "enc-1", "enc-2", "diff");	
			for (i=1; i<29000-1; i++)
			{
				e1=_Val[0][i];
				e2=_Val[1][i];
				if (e1<e2) e1+=0x10000;
				diff = e1-e2;
				if (i==1) base = diff;
				fprintf(f, "%6d %6d %6d %6d\n", i, e1, e2, diff-base);
			}
			fclose(f);			
		}
	}
#endif
}
//--- test_write_csv ------------------------------------
void test_write_csv_print_go(void) // Print-Go
{
#ifdef DEBUG
	if (_Val[0])
	{
		int i, n;
		int diff;
		int base;
		int e1, e2;
		FILE *f;
		f = fopen(PATH_TEMP "encoder_test.csv", "w");
		if (f!=NULL)
		{
			fprintf(f, "%6s %6s %6s %6s\n", "#", "enc-1", "enc-2", "diff");	
			for (i=0; i<10*8; i++)
			{
				fprintf(f, "%06d", _Val[2][i+1]);
				if (i%8==7) fprintf(f, "\n"); 
				else		fprintf(f, "\t"); 
			}
			fclose(f);			
		}
	}
#endif
}


//--- test_speed ------------------------------------
void test_speed(int ticks)
{
	extern SEncFpga		*Fpga;
	static int running=FALSE;
	double fact = FPGA_FREQ / 101455920.0 / 1000000.0 * 60;
	
//	int min = Fpga->stat.encOut[0].speed_min;
	if (Fpga->cfg.encIn[0].enable)
	{
		if (Fpga->stat.encOut[0].speed>50000 || running)
		{
			static FILE *f=NULL;
			running = TRUE;
			if (f==NULL)
			{
				f = fopen(PATH_TEMP "step_time.csv", "w");
				fprintf(f, "PG\tmin\tmax\tact\n");
			}
			if (f!=NULL)
			{
				fprintf(f, "PG\tmin\tmax\tact\n");
//				fprintf(f, "%d\t%d\t%d\t%d\t%d\n", ticks-start, Fpga->stat.encOut[0].speed, (int)(Fpga->stat.encOut[0].speed*0.021/50000.0*60.0),
//														Fpga->stat.encOut[4].speed, (int)(Fpga->stat.encOut[4].speed*0.021/50000.0*60.0));
				fflush(f);			
			}			
		}
	}
	else running=FALSE;
}

//--- test_step_time ------------------------------------
void test_step_time(int ticks)
{
	extern SEncFpga		*Fpga;
	static int running=FALSE;
	static int pg=0;
	double fact = FPGA_FREQ / 101455920.0 / 1000000.0 * 60;
	
	if (Fpga!=NULL && Fpga->cfg.encIn[0].enable)
	{
		static FILE *f=NULL;
		running = TRUE;
		if (f==NULL)
		{
			Error(WARN, 0, "ENCODER STEP TIME TRACING");
			f = fopen(PATH_TEMP "step_time.csv", "w");
			fprintf(f, "PG\tmin\tmax\tact\tspeed\tm/min\n");
		}
		if (f!=NULL)
		{
			if (pg!=Fpga->stat.encOut[0].PG_cnt) 
			{
				Fpga->cfg.encIn[0].reset_min_max=1;
				pg = Fpga->stat.encOut[0].PG_cnt;
			}
			fprintf(f, "%d\t%d\t%d\t%d\t%d\t%d\n", Fpga->stat.encOut[0].PG_cnt, Fpga->stat.encIn[0].setp_time_min, Fpga->stat.encIn[0].step_time_max, Fpga->stat.encIn[0].step_time, Fpga->stat.encOut[0].speed, (int)(Fpga->stat.encOut[0].speed*fact));
			fflush(f);			
		}			
	}
	else running=FALSE;
}
