// ****************************************************************************
//
//	DIGITAL PRINTING - fpga_enc.c
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2015 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#ifdef linux
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/mman.h>
	#include <unistd.h>
#endif
#include "args.h"
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include "rx_common.h"
#include "rx_error.h"
#include "rx_fpga.h"
#include "rx_file.h"
#include "rx_mac_address.h"
#include "rx_term.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "args.h"
#include "enc_ctrl.h"
#include "enc_test.h"
#include "tw8.h"
#include "fpga_def_enc.h"
#include "fpga_enc.h"

//---- COMPILER OPTIONS ------------------------------------------------------

static int _ALL=FALSE;	// show all registers

//--- defines -----------------------------------------------------------------
//--- sub zero power levels ----------------- 
//	0x00:	 0%	Do not use
//	0x01:	26% Do not use
//	0x03:	40% Lowest working level
//	0x02:	50%
//	0x06:	60%
//	0x07:	70%
//	0x05:	80% 
//	0x04:  100%

#define WATCHDOG_CNT	0x7fffffff

#define UV_ISOLATOR_OUT	0x0001		
#define UV_POWER_OUT	0x0002	// Power-Bits= 0x04
#define UV_SHUTTER_OUT	0x0004

#define UV_FAULT_IN		0x0004
#define UV_READY_IN		0x0001

#define UV_POS_LEFT		100000
#define UV_POS_RIGHT	550000

//--- structures ----------------------------

static UINT32 _AVR_COEFF [][4]=
	{
		// coeff1| coeff2	| coeff3		
		{	8,		0x06,		0x0008},	// 0
		{  16,		0x08,		0x0010},	// 1
		{  32,		0x0a,		0x0020},	// 2
		{  64,		0x0c,		0x0040},	// 3
		{ 128,		0x0e,		0x0080},	// 4
		{ 256,		0x10,		0x0100},	// 5	
		{ 512,		0x12,		0x0200},	// 6
		{1024,		0x14,		0x0400},	// 7
		{2048,		0x16,		0x0800},	// 8
		{4069,		0x18,		0x1000},	// 9
		{8192,		0x1a,		0x2000}		// 10
	};

#define AVR_COEFF_IDX	4

static INT32 *_CorrVal[2];
static INT32 *_CorrSin;
static INT32 *_CorrSinRev;

//--- globals -----------------------------------------------------------------

SEncFpga		*Fpga;
SEncFpgaQSys	*FpgaQSys;
SEncFpgaCorr	*FpgaCorr;	

//--- module globals ---------------------------------------
static int		_MemId=0;
static int		_Init=FALSE;

static double	_IncDist	= 1.0040;			// [µm] distance of encoder increments
static double   _StrokeDist = 25400.0/1200.0;	// [µm] distance between firepulse strokes

static int		_UV_Speed   = 0;
static INT32	_UV_LastPos = 0;
static int		_UV_Stopping= FALSE;
static int		_UV_Printing= FALSE;
static int		_UV_BiDir	= FALSE;
static int		_UV_Power	= 0;
static int		_UV_Shutter = 0;
static int		_UV_SimuCnt = 0;
static int		_PrintGo_Locked;
static int		_DistTelCnt = 0;
static int		_Scanning = FALSE;
static int		_IndexCheckDone=FALSE;
static UINT32	_ErrFlags	= 0;
static UINT32	_PG_WindowError = 0;
static RX_SOCKET _Socket = INVALID_SOCKET;
	
//--- prototypes -------------------------------------------
static int  _fpga_running(void);
static void _fpga_display_status_test(void);
static void _fpga_display_status(int showCorrection);
static void _fpga_display_error(void);
static void _fpga_corr_rotative(SEncoderCfg *pcfg);
static void _fpga_corr_linear(SEncoderCfg *pcfg);

static void  _uv_init(void);
static void  _uv_ctrl(void);
static void  _pg_ctrl(void);
static void  _corr_ctrl(void);

static void  _check_errors(void);


//--- scan check -----------------------------------------------------------
static UINT32 _scanpos_last;
static UINT32 _scanpos_left;
static UINT32 _scanpos_right;
static UINT32 _scanpos_cnt;
static UINT32 _scanpos_pg;
static UINT32 _PG_cnt;
static UINT32 _scanpos_fwd;

static void  _scan_check_init(void);
static void	 _scan_check(void);

// static void _LoadSinus(char *filepath, INT32 *pmem, int size);
static void _CalcSinus(int max, int cnt, INT32 *pmem, int size);

//*** functions ********************************************

//--- fpga_init ----------------
void fpga_init()
{
	_Init = FALSE;
	if (rx_fpga_load (PATH_BIN_ENCODER FIELNAME_ENCODER_RBF)) return;

#ifdef soc	
	//--- map the meory ------------------
	_MemId = open("/dev/mem", O_RDWR | O_SYNC);
	if (_MemId==-1) Error(ERR_CONT, 0, "Could not open memory handle.");

	UINT32 *pid;
	UINT32 test;
	
	_CorrVal[0] = (INT32*)rx_fpga_map_page(_MemId, ENC_CORR_VALUES_0, 0x1000*sizeof(INT32), 0x1000*sizeof(INT32));
	_CorrVal[1] = (INT32*)rx_fpga_map_page(_MemId, ENC_CORR_VALUES_1, 0x1000*sizeof(INT32), 0x1000*sizeof(INT32));

	_CorrSin	= (INT32*)rx_fpga_map_page(_MemId, SINUS_CORRECTION,     0x10000*sizeof(INT32), 0x10000*sizeof(INT32));
	_CorrSinRev = (INT32*)rx_fpga_map_page(_MemId, SINUS_IDENTIFICATION, 0x10000*sizeof(INT32), 0x10000*sizeof(INT32));

	if (sizeof(SEncFpga)!=0x0e40)
	{
		SEncFpga test;
		term_printf("TEST Structures -------\n");
		term_printf("info				@ 0x%04x (0x0400)\n", (BYTE*)&test.stat.info    -(BYTE*)&test);
		term_printf("drive_err			@ 0x%04x (0x0494)\n", (BYTE*)&test.stat.drive_err -(BYTE*)&test);
		term_printf("dig_pg_window_err  @ 0x%04x (0x04d4)\n", (BYTE*)&test.stat.dig_pg_window_err -(BYTE*)&test);
		term_printf("in-cfg				@ 0x%04x (0x0800)\n", (BYTE*)&test.cfg.encIn[0] -(BYTE*)&test);
		term_printf("out-cfg			@ 0x%04x (0x0a00)\n", (BYTE*)&test.cfg.encOut[0]-(BYTE*)&test);
		term_printf("pg-cfg				@ 0x%04x (0x0c00)\n", (BYTE*)&test.cfg.pg		 -(BYTE*)&test);
		term_printf("general			@ 0x%04x (0x0d00)\n", (BYTE*)&test.cfg.general	 -(BYTE*)&test);
		term_flush();
		Error(ERR_ABORT, 0, "structuire mismatch");
	}

	Fpga = (SEncFpga*)rx_fpga_map_page(_MemId, 0xc0200000, sizeof(SEncFpga), 0x0e58);
	FpgaCorr = (SEncFpgaCorr*)rx_fpga_map_page(_MemId, 0xc0203000, sizeof(SEncFpgaCorr),	0x600);

	{
		int outNo=0;

		term_printf("dist_ratio:	 %d\n", Fpga->cfg.encOut[outNo].dist_ratio);
		term_printf("synthetic_freq: %d\n", Fpga->cfg.encOut[outNo].synthetic_freq);
		term_printf("backlash:       %d\n", Fpga->cfg.encOut[outNo].backlash);
		term_printf("scanning:       %d\n", Fpga->cfg.encOut[outNo].scanning);	
		term_flush();
	}
	Fpga->cfg.general.watchdog_freq = 250000;
	Fpga->cfg.general.watchdog_cnt  = WATCHDOG_CNT;

	{
		FpgaQSys = (SEncFpgaQSys*)rx_fpga_map_page(_MemId, 0xff200000, sizeof(SEncFpgaQSys),	0x00d0);
		
		/*
		{
			int i;
			pid = (UINT32*)FpgaQSys;
			for (i=0; i<20; i++)
			{
				TrPrintf(TRUE, "mem[0x%02x]=0x%08x", i*4, pid[i]);						
			}
		}
		pid = (UINT32*)&mem[0x08];
		test=*pid,
		term_printf("SysID=0x%08x", *pid);
		*/
	}
	test_init();
	
	_Init = TRUE;
#endif
}

// --- fpga_end -------------------
void fpga_end()
{
#ifdef linux
	int i;
	
	_Init = FALSE;
	
	for (i=0; i<8; i++) Fpga->cfg.encOut[i].synthetic_freq	= 0;
	
	if (munmap(Fpga,   sizeof(*Fpga))) Error(ERR_CONT, 0, "Could not UNMAP Fpga.stat ");

	close (_MemId);
	_MemId = 0;
#endif
}

//--- fpga_main ------------------------------------------
void  fpga_main(int ticks, int menu, int showCorrection)
{	
	static int _lastTicks=0;
	if (_Init)
	{
		_check_errors();
		_uv_ctrl();
		_pg_ctrl();
		_corr_ctrl();		
	}

//	_scan_check();
	//--- TEST ---------------------------
	/*
	if (TRUE)
	{
		static int _ticks=0;
		static FILE *file=NULL;
		if (ticks-_ticks>10)
		{
			if (Fpga->stat.encOut[0].PG_cnt)
			{
				if (file==NULL)
				{
					file = fopen(PATH_TEMP "encoder_test.csv", "w");
					fprintf(file, "%6s\t%6s\t%6s\t%6s\t%6s\n", "pos1", "pg1", "pos2", "pg2");
				}
				fprintf(file, "%6d\t%6d\t%6d\t%6d\n", Fpga->stat.encIn[0].position,  Fpga->stat.encOut[0].PG_cnt,
												   Fpga->stat.encIn[1].position,  Fpga->stat.encOut[1].PG_cnt);
				fflush(file);				
			}
			_ticks=ticks;
		}
	}
	*/
	//------------------------------------
	if (ticks-_lastTicks>10)
	{
		_lastTicks = ticks;
//		test_speed(ticks);
//		test_step_time(ticks);
		test_do(ticks);
	}
		
	if (_Init) Fpga->cfg.general.watchdog_cnt  = WATCHDOG_CNT;
	if (menu)
	{
		test_do(ticks);
		if (arg_test) _fpga_display_status_test();
		else		  _fpga_display_status(showCorrection);
		if (arg_simu_uv && RX_EncoderStatus.info.uv_on)
		{
			_UV_SimuCnt++;
		}
	}
}

//--- fpga_output --------------------------------
void  fpga_output(int no)
{
	FpgaQSys->out ^= (1<<no);	
}

//--- fpga_uv_on ------------------------------------
void  fpga_uv_on(void)
{
	_UV_Power	  = UV_POWER_OUT;
	FpgaQSys->out = UV_ISOLATOR_OUT | _UV_Power;
	_UV_SimuCnt=0;
}

//--- fpga_uv_off -------------------------------------
void  fpga_uv_off(void)
{
	_UV_Power	  = 0;
	FpgaQSys->out = 0x0000;
}

//--- _uv_init --------------------------------------
static void _uv_init(void)
{
	_UV_Speed   = 0;
	_UV_LastPos = Fpga->stat.encIn[0].position;
	_UV_Shutter = 0;
	_UV_Stopping = FALSE;
	_UV_Printing = TRUE;
	_UV_BiDir	 = FALSE;
}

//--- _uv_ctrl --------------------------------------------------------
static void  _uv_ctrl(void)
{
	int actPos;
	int shutter = FALSE;
	
	if (!_Init) return;
	
	RX_EncoderStatus.info.uv_on		= (FpgaQSys->out & UV_POWER_OUT)!=0;
	if (arg_simu_uv) RX_EncoderStatus.info.uv_ready = _UV_SimuCnt>5;
	else             RX_EncoderStatus.info.uv_ready  = (FpgaQSys->in  & UV_READY_IN)!=0;
	
	if (Fpga->cfg.encIn[0].enable)
	{
		actPos = Fpga->stat.encIn[0].position;
		if (_UV_Stopping && actPos<UV_POS_LEFT) 
		{
			_UV_Stopping = FALSE;
			_UV_Printing = FALSE;
		}
		_UV_Speed = actPos- _UV_LastPos;
		_UV_LastPos = actPos;
		if (_UV_Printing && actPos > UV_POS_LEFT && actPos < UV_POS_RIGHT)
		{
			if (_UV_BiDir || _UV_Speed<0) shutter=TRUE;
		}
	}
	else 
	{
		_UV_LastPos= 0;
		_UV_Speed  = 0;
	}

	if (_UV_Shutter!=shutter && _UV_Power)
	{
		if	(shutter)	FpgaQSys->out = UV_ISOLATOR_OUT | _UV_Power | UV_SHUTTER_OUT;
		else 			FpgaQSys->out = UV_ISOLATOR_OUT | _UV_Power;
		_UV_Shutter=shutter;
	}	
}

//--- fpga_stop_printing ------------------------ 
void fpga_stop_printing(void)
{
//	_UV_Stopping = TRUE;
//	_UV_BiDir    = TRUE;
	if (_Init)
	{
		int pgNo;
		for (pgNo=0; pgNo<SIZEOF(Fpga->cfg.pg); pgNo++)
		{
			Fpga->cfg.pg[pgNo].enc_start_pos_fwd =   10000*1000;
			Fpga->cfg.pg[pgNo].enc_start_pos_bwd =   10000*1000; 							
		}
	}
}

//--- fpga_abort_printing ------------------------ 
void fpga_abort_printing(void)
{
	int i;
	_UV_Stopping = TRUE;
	for (i=0; i<SIZEOF(Fpga->cfg.encIn); i++) Fpga->cfg.encIn[i].enable = FALSE;
}

//--- fpga_enc_config ---------------------------------------------------
void fpga_enc_config(int inNo, SEncoderCfg *pCfg, int outNo, int khz)
{
	int i;
	
	if (!_Init) return;

	_Scanning = rx_def_is_scanning(pCfg->printerType);

	if (pCfg->printerType==printer_TX801 || pCfg->printerType==printer_TX802 
	||  pCfg->printerType==printer_LB701 
	||  pCfg->printerType==printer_LB702_UV || pCfg->printerType==printer_LB702_WB
	||  pCfg->printerType==printer_DP803	) 
	{
		if(!tw8_present()) Error(ERR_CONT, 0, "No Communication to analog encoder board");
	}
	
	tw8_config(0, pCfg->speed_mmin, pCfg->printerType);
	tw8_config(1, pCfg->speed_mmin, pCfg->printerType);
		
	if (khz)
	{
		double freq;

		_IncDist = _StrokeDist; // [µm] distance of encoder increments
		
		for (outNo=0; outNo<8; outNo++)
		{
			Fpga->cfg.encOut[outNo].encoder_no		= 0x08;
			Fpga->cfg.encOut[outNo].reset_min_max	= TRUE;
			Fpga->cfg.encOut[outNo].dist_ratio		= 0x80000000;
			if (khz == 1) freq = 0xffffffff;
			else		  freq = 50000.0 * 8.0 / khz;	// 8 Subpulses!
			Fpga->cfg.encOut[outNo].synthetic_freq	= (UINT32)freq;
			Fpga->cfg.encOut[outNo].backlash		= FALSE;
			Fpga->cfg.encOut[outNo].scanning		= FALSE;				
		}
		for (i=0; i<SIZEOF(Fpga->cfg.pg); i++)
		{	
			//--- reset ------------------
			Fpga->cfg.encIn[i].reset_pos		= 0;
			Fpga->cfg.pg[i].reset_pos			= FALSE;
			Fpga->cfg.pg[i].reset_pos			= TRUE;
			Fpga->cfg.pg[i].reset_pos			= FALSE;
			Fpga->cfg.pg[i].enc_start_pos_fwd	= 100;
			Fpga->cfg.pg[i].pos_pg_fwd			= 10;
		}
	}
	else
	{	
		_IncDist = 1000000.0/pCfg->incPerMeter; // [µm] distance of encoder increments
		
	//	if (tw8_present()) _IncDist *= 2;
	
		Fpga->cfg.encIn[inNo].enable			= FALSE;
		
		Fpga->cfg.encIn[inNo].reset_min_max		= TRUE;
		Fpga->cfg.encIn[inNo].orientation		= pCfg->orientation;
		Fpga->cfg.encIn[inNo].index_en			= FALSE;			
		Fpga->cfg.encIn[inNo].index_on_b		= FALSE;
		Fpga->cfg.encIn[inNo].inc_per_revolution= 0xffffffff;
		Fpga->cfg.encIn[inNo].correction		= pCfg->correction;
		
		Fpga->cfg.encOut[outNo].encoder_no		= inNo;
		Fpga->cfg.encOut[outNo].reset_min_max	= TRUE;
		Fpga->cfg.encOut[outNo].dist_ratio		= (UINT32) ((double) 0x80000000 * _IncDist / _StrokeDist);
		Fpga->cfg.encOut[outNo].synthetic_freq	= 0;
		Fpga->cfg.general.subsample_meas		= 0;
		if (pCfg->scanning)
		{
			Fpga->cfg.encOut[outNo].backlash	= TRUE;
			Fpga->cfg.encOut[outNo].scanning	= TRUE;
		}
		else
		{
			Fpga->cfg.encIn[inNo].index_en		= TRUE;		
			Fpga->cfg.encOut[outNo].backlash	= TRUE;
			Fpga->cfg.encOut[outNo].scanning	= FALSE;				
			Fpga->cfg.encIn[inNo].inc_per_revolution= 238000;
		}
//		Fpga->cfg.encOut[outNo].calculationEndable = TRUE;				

		switch(pCfg->correction)
		{
		case 	CORR_ROTATIVE:	if (inNo==0) _fpga_corr_rotative(pCfg); break;
		case 	CORR_LINEAR:	if (inNo==0) _fpga_corr_linear(pCfg);  break;
		default:	break;
		}
		RX_EncoderStatus.meters = 0;
		_corr_ctrl();		
		Fpga->cfg.encIn[inNo].enable			= TRUE;
	}
	_uv_init();
//	_scan_check_init();
	_ErrFlags = 0;
	_PG_WindowError = 0;
	_DistTelCnt=0;
	_IndexCheckDone = FALSE;
}

//--- fpga_encoder_enable -------------------------------------
void  fpga_encoder_enable(int enable)
{
	int i;
	
	if (_Init)
	{
		for(i=0; i<2; i++)
		{
			Fpga->cfg.encOut[i].reset_min_max	= TRUE;
			Fpga->cfg.encIn[i].enable		= enable;		
			Fpga->cfg.encIn[i].index_en		= enable;				
		//	Fpga->cfg.encIn[i].correction	= FALSE;

		}
	}

//	Fpga->cfg.general.max_a0_var_high		= 1000;		// 0x0020: Threshold for the variation value, when to switch off correction
//	Fpga->cfg.general.max_a0_var_low		= 100;		// 0x0024: Threshold for the variation value, when to switch on correction
//	
//	Fpga->cfg.general.ident_rol_en			= TRUE;		// FALSE; //TRUE;			// 0x0018: enables coeff. identification logic in roller correction
//	Fpga->cfg.general.single_sin_en			= FALSE;	// 0x0040: enables use of apm and phase instead of b1 and a1
//	Fpga->cfg.general.use_internal_ident_en	= FALSE; //FALSE;	// 0x0044: enables use of ident values
//	Fpga->cfg.general.runnung_avg_coeff		= TRUE;		// 0x0048: disables coeff running average
//	Fpga->cfg.general.rol_2_first			= FALSE;	// 0x004C: flag
//	Fpga->cfg.general.rol_drift_mu_two		= 0x0;		// 0x0050:
//	Fpga->cfg.general.rol_ident_res_shift	= 0x03; // 0x01; // 0x07; // f(inc_per_revolution)
//	//Error(WARN, 0, "calculate rol_ident_res_shift");
//	
//	// b1 ident 303222=4A076 -> 0x0002CE49
//	// a1 ident 183881=2CE49 -> 0xFFFB5F89
//	Fpga->cfg.general.rol_0_new_b1			= 0x0002CE49; // 0x001E0000; // 0x000F0000; // 0x02D1C;	// 0x70000;	// 0x0028: a0 sin coeff for roller 0 correction, overwrites internal coeff !
//	Fpga->cfg.general.rol_0_new_a1			= 0xFFFB5F89; // 0x0012E;	// 0x30000;	// 0x002c: a1 cos coeff for roller 0 correction, overwrites internal coeff !
//	Fpga->cfg.general.rol_1_new_b1			= 0x00000000; // 0x001E0000; // 0x000F0000; // 0xFFFF1D74;	// 0x80000;	// 0x0030: a0 sin coeff for roller 1 correction, overwrites internal coeff !
//	Fpga->cfg.general.rol_1_new_a1			= 0x00000000; // 0x001DC;	// 0x20000;	// 0x0034: a1 cos coeff for roller 1 correction, overwrites internal coeff !
}		

//--- _CalcSinus -------------------------------------
static void _CalcSinus(int max, int cnt, INT32 *pmem, int size)
{
	int i;
	cnt /= 4;
	max++;
	memset(pmem, 0, size*sizeof(int));
	for (i=0; i<cnt; i++)
	{	
		pmem[i]=(int)((double)max*sin(M_PI_2*i/cnt));		
	}	
}

//--- _trace_sinus ---------------------------------
static void _trace_sinus(int cnt, INT32* pmem, const char* filepath)
{
	FILE *file;
	file=fopen(filepath, "w");
	if (file)
	{
		int i;
		for (i=0; i<cnt/4; i++)
		{
			fprintf(file, "%d\n", pmem[i]);
		}		
		fclose(file);			
	}
}

//--- _fpga_corr_rotative ------------------------
static void _fpga_corr_rotative(SEncoderCfg *pCfg)
{
//	_trace_sinus(Fpga->cfg.encIn[0].inc_per_revolution, _CorrSin,	 PATH_TEMP "CorrSin1.csv");
//	_trace_sinus(Fpga->cfg.encIn[0].inc_per_revolution, _CorrSinRev, PATH_TEMP "CorrSinRev1.csv");
	
//	_CalcSinus(0x8000,										Fpga->cfg.encIn[0].inc_per_revolution, _CorrSin,    0x10000);
//	_CalcSinus(Fpga->cfg.encIn[0].inc_per_revolution/1000,  Fpga->cfg.encIn[0].inc_per_revolution, _CorrSinRev, 0x10000);

//	_trace_sinus(Fpga->cfg.encIn[0].inc_per_revolution, _CorrSin,	 PATH_TEMP "CorrSin2.csv");
//	_trace_sinus(Fpga->cfg.encIn[0].inc_per_revolution, _CorrSinRev, PATH_TEMP "CorrSinRev.csv");
	
	fpga_enc_config(1, pCfg, 1, 0);
	
	Error(WARN, 0, "Warning: Testsoftware for Roller Correction enabled");
	
	Fpga->cfg.general.max_a0_var_high		= 1000;		// 0x0020: Threshold for the variation value, when to switch off correction
	Fpga->cfg.general.max_a0_var_low		= 100;		// 0x0024: Threshold for the variation value, when to switch on correction
	
	Fpga->cfg.general.ident_rol_en			= TRUE;		// FALSE; //TRUE;			// 0x0018: enables coeff. identification logic in roller correction
	Fpga->cfg.general.single_sin_en			= FALSE;	// 0x0040: enables use of apm and phase instead of b1 and a1
	Fpga->cfg.general.use_internal_ident_en	= FALSE;	// 0x0044: enables use of ident values
	Fpga->cfg.general.runnung_avg_coeff		= TRUE;		// 0x0048: disables coeff running average
	Fpga->cfg.general.rol_2_first			= FALSE;	// 0x004C: flag
	Fpga->cfg.general.rol_drift_mu_two		= 0x0;		// 0x0050:
	Fpga->cfg.general.rol_ident_res_shift	= 0x03; // 0x01; // 0x07; // f(inc_per_revolution)
	
	// Cheseaux Label UV701-004
	// b1 ident 303222=4A076 -> 0xFFFD31B6	-183882
	// a1 ident 183881=2CE49 -> 0x0004A076	303222
		
	Fpga->cfg.general.rol_0_new_b1			= 0xFFFFFFFF - pCfg->corrRotPar[0]; // 0x001E0000; // 0x000F0000; // 0x02D1C;	// 0x70000;	// 0x0028: a0 sin coeff for roller 0 correction, overwrites internal coeff !
	Fpga->cfg.general.rol_0_new_a1			= pCfg->corrRotPar[1]; // 0x0012E;	// 0x30000;	// 0x002c: a1 cos coeff for roller 0 correction, overwrites internal coeff !
//	Fpga->cfg.general.rol_1_new_b1			= 0x00000000; // 0x001E0000; // 0x000F0000; // 0xFFFF1D74;	// 0x80000;	// 0x0030: a0 sin coeff for roller 1 correction, overwrites internal coeff !
//	Fpga->cfg.general.rol_1_new_a1			= 0x00000000; // 0x001DC;	// 0x20000;	// 0x0034: a1 cos coeff for roller 1 correction, overwrites internal coeff !

//	Fpga->cfg.general.rol_0_new_b1			= 0x00000000; // 0x001E0000; // 0x000F0000; // 0x02D1C;	// 0x70000;	// 0x0028: a0 sin coeff for roller 0 correction, overwrites internal coeff !
//	Fpga->cfg.general.rol_0_new_a1			= 0x00000000; // 0x0012E;	// 0x30000;	// 0x002c: a1 cos coeff for roller 0 correction, overwrites internal coeff !
	Fpga->cfg.general.rol_1_new_b1			= 0x00000000; // 0x001E0000; // 0x000F0000; // 0xFFFF1D74;	// 0x80000;	// 0x0030: a0 sin coeff for roller 1 correction, overwrites internal coeff !
	Fpga->cfg.general.rol_1_new_a1			= 0x00000000; // 0x001DC;	// 0x20000;	// 0x0034: a1 cos coeff for roller 1 correction, overwrites internal coeff !

//	Fpga->cfg.general.ident_rol_en			= TRUE;		// FALSE; //TRUE;			// 0x0018: enables coeff. identification logic in roller correction
//	Fpga->cfg.general.single_sin_en			= TRUE;	// 0x0040: enables use of apm and phase instead of b1 and a1
//	Fpga->cfg.general.use_internal_ident_en	= FALSE; //FALSE;	// 0x0044: enables use of ident values
//	Fpga->cfg.general.runnung_avg_coeff		= TRUE;		// 0x0048: disables coeff running average
//	Fpga->cfg.general.rol_2_first			= FALSE;	// 0x004C: flag
//	Fpga->cfg.general.rol_drift_mu_two		= 0x0;		// 0x0050:
//	Fpga->cfg.general.rol_ident_res_shift	= 0x03; // 0x01; // 0x07; // f(inc_per_revolution)
//	
//	// b1 ident 303222=4A076 -> 0xFFFD31B6
//	// a1 ident 183881=2CE49 -> 0x0004A076
//	// Amplitude 354621 -> 5693D
//	// Phase arcsin(-183881/Amplitude)=-31 grad für cosi -> -121 grad für sin -> 238 grad für sin -> /360*238000 -> 157851 steps
//	Fpga->cfg.general.rol_0_new_b1			= 0x0005693D; // 0x001E0000; // 0x000F0000; // 0x02D1C;	// 0x70000;	// 0x0028: a0 sin coeff for roller 0 correction, overwrites internal coeff !
//	Fpga->cfg.general.rol_0_new_a1			= 0x0002689B; // 0x0012E;	// 0x30000;	// 0x002c: a1 cos coeff for roller 0 correction, overwrites internal coeff !
//	Fpga->cfg.general.rol_1_new_b1			= 0x00000000; // 0x001E0000; // 0x000F0000; // 0xFFFF1D74;	// 0x80000;	// 0x0030: a0 sin coeff for roller 1 correction, overwrites internal coeff !
//	Fpga->cfg.general.rol_1_new_a1			= 0x00000000; // 0x001DC;	// 0x20000;	// 0x0034: a1 cos coeff for roller 1 correction, overwrites internal coeff !

}

//--- _fpga_corr_linear ------------------------
static void _fpga_corr_linear(SEncoderCfg *pCfg)
{
	double dist_encoders = 472000;
	double dist_head	 = 48768;
	double offset;
	INT32 val;
	int i;
	
	fpga_enc_config(1, pCfg, 1, 0);
	
	_IncDist = 1000000.0/pCfg->incPerMeter; // [µm] distance of encoder increments

	double ratio = _IncDist / _StrokeDist;

	UINT32 rat = (UINT32) (0x80000000 * ratio);

	offset = (dist_encoders - 7*dist_head)/2;
	for (i=0; i<ENCODER_OUT_CNT; i++) Fpga->cfg.encIn[i].enable	= FALSE;
	for (i=0; i<ENCODER_OUT_CNT; i++)
	{
		Fpga->cfg.encIn[i].enable				= FALSE;
		Fpga->cfg.encIn[i].correction			= CORR_LINEAR; 
//		Fpga->cfg.encOut[i].encoder_no			= 0;
		Fpga->cfg.encOut[i].encoder_no			= i;
		Fpga->cfg.encOut[i].reset_min_max		= TRUE;
		Fpga->cfg.encOut[i].dist_ratio			= rat;
		Fpga->cfg.encOut[i].synthetic_freq		= 0;
		Fpga->cfg.encOut[i].backlash			= FALSE;
		Fpga->cfg.encOut[i].scanning			= TRUE;
	//	Fpga->cfg.encOut[i].calculationEndable	= TRUE;	
		FpgaCorr->par[i].par = val=(INT32)(0x8000*(offset+i*dist_head)/dist_encoders);
	}	
	for (i=0; i<ENCODER_OUT_CNT; i++) Fpga->cfg.encIn[i].enable	= TRUE;
}

//--- fpga_enc_config_test ---------------------------------------------
void  fpga_enc_config_test(void)
{
	if (!_Init) return;

	int i;
	for (i=0; i<SIZEOF(Fpga->cfg.encIn); i++)
	{
		Fpga->cfg.encIn[i].enable				= FALSE;
		Fpga->cfg.encIn[i].reset_pos			= 0;
		Fpga->cfg.pg[0].reset_pos				= TRUE;
		Fpga->cfg.pg[0].reset_pos				= FALSE;
		Fpga->cfg.pg[0].pos_pg_fwd				= 10;
		Fpga->cfg.encIn[i].reset_min_max		= TRUE;
		Fpga->cfg.encIn[i].orientation			= FALSE;
		Fpga->cfg.encIn[i].index_on_b			= FALSE;
		Fpga->cfg.encIn[i].inc_per_revolution	= 0xffffffff;
		Fpga->cfg.encIn[i].correction			= 0;
		Fpga->cfg.encIn[i].enable				= TRUE;
	}
}

//--- _micron2inc ------------------------------------------------
static UINT32 _micron2inc(UINT32 mircons)
{
	return (UINT32)((double)mircons/_IncDist);
}

//--- fpga_pg_config_fhnw -----------------------------------------
int  fpga_pg_config_fhnw(UINT32 posActual, UINT32 posFwd, UINT32 posBwd)
{
	if (!_Init) return REPLY_ERROR;
			
//	Fpga->cfg.encIn[0].reset_pos   = _micron2inc(posActual);
	Fpga->cfg.encIn[0].orientation = FALSE;

	Fpga->cfg.pg[0].reset_pos		   = TRUE;
	Fpga->cfg.pg[0].reset_pos		   = FALSE;
	Fpga->cfg.pg[0].enc_start_pos_fwd  =   10*1000;
	Fpga->cfg.pg[0].pos_pg_fwd		   =   10*1000;
	
	Fpga->cfg.pg[0].enc_start_pos_bwd  = 1200*1000;
	Fpga->cfg.pg[0].pos_pg_bwd		   = 1000*1000; // no print-go on backwards
	
	Fpga->cfg.pg[0].fifos_used		   = FALSE;
	Fpga->cfg.pg[0].printgo_n		   = TRUE;
	return REPLY_OK;
}

//--- fpga_pg_config -----------------------------------------
int  fpga_pg_config(RX_SOCKET socket, SEncoderCfg *pcfg)
{
	_Socket = socket;

	int pgNo;
	
	//--- reset  position ------------------
	int act_pos =_micron2inc(pcfg->pos_actual);
//	if ((int)Fpga->stat.encIn[0].position - act_pos > 1000 || (int)Fpga->stat.encIn[0].position - act_pos < -1000)	
	{
		for (pgNo=0; pgNo<SIZEOF(Fpga->cfg.pg); pgNo++)
		{
			Fpga->cfg.pg[pgNo].reset_pos	= FALSE;			
			Fpga->cfg.encIn[pgNo].reset_pos = act_pos;
		}				
	}
	for (pgNo=0; pgNo<SIZEOF(Fpga->cfg.pg); pgNo++)
	{		
		//--- set positions ---------------
		if (pcfg->pos_pg_fwd<100) 
			Error(ERR_ABORT, 0, "ERROR Position");
		Fpga->cfg.pg[pgNo].enc_start_pos_fwd = _micron2inc(pcfg->pos_pg_fwd-100);
		Fpga->cfg.pg[pgNo].pos_pg_fwd		 = 10;
		Fpga->cfg.pg[pgNo].enc_start_pos_bwd = _micron2inc(pcfg->pos_pg_bwd+100);
		Fpga->cfg.pg[pgNo].pos_pg_bwd		 = 10;
		Fpga->cfg.pg[pgNo].reset_pos		 = TRUE;
		Fpga->cfg.pg[pgNo].reset_pos		 = FALSE;			

		if (pcfg->scanning)
		{
			Fpga->cfg.pg[pgNo].fifos_used   = FALSE;		
		}
		else 
		{
			Fpga->cfg.pg[pgNo].fifos_used = TRUE;
			if (FpgaQSys->printGo_status.fill_level) Fpga->cfg.pg[pgNo].fifos_ready	= TRUE;
		}
	
	//	Fpga->cfg.pg.ext_printgo	   = FALSE;
		Fpga->cfg.pg[pgNo].printgo_n	= TRUE;
	}
	RX_EncoderStatus.PG_cnt		   = 0;
	_PrintGo_Locked				   = FALSE;

	test_cfg_done();
	return REPLY_OK;
}

//--- fpga_pg_init ---------------------------------
void fpga_pg_init(void)
{
	int pgNo;
	int tio=0;
	
	TrPrintfL(TRUE, "fpga_pg_init");

	TrPrintfL(TRUE, "fpga_pg_init: level=%d", FpgaQSys->printGo_status.fill_level);
	
	_PrintGo_Locked = TRUE;
	RX_EncoderStatus.PG_cnt = 0;
	for (pgNo=0; pgNo<SIZEOF(Fpga->cfg.pg); pgNo++)
	{
		Fpga->cfg.pg[pgNo].fifos_ready = FALSE;
		Fpga->cfg.pg[pgNo].fifos_used  = FALSE;				
	}
	
	Fpga->cfg.general.reset_fifos = TRUE;
	do
	{
		Fpga->cfg.general.reset_fifos = TRUE;
		rx_sleep(10);
		tio += 10;
		TrPrintfL(TRUE, "fpga_pg_init: reset_fifos level=%d, time=%d", FpgaQSys->printGo_status.fill_level, tio);
		if (tio>100) 
		{
			Error(WARN, 0, "Reset PG-FIFO level=%d", FpgaQSys->printGo_status.fill_level);
			fpga_init();
			break;
		}
	}
	while (FpgaQSys->printGo_status.fill_level);
	TrPrintfL(TRUE, "fpga_pg_init done");
}

//--- fpga_pg_set_dist -------------------------------------------
void  fpga_pg_set_dist(int cnt, int dist)
{
	int pgNo;
	dist = (int)(dist/_StrokeDist);
	while (cnt-->0) FpgaQSys->printGo_fifo = dist;
	for (pgNo=0; pgNo<SIZEOF(Fpga->cfg.pg); pgNo++) Fpga->cfg.pg[pgNo].fifos_ready = TRUE;
}

//--- fpga_set_printmark -------------------------------------------
void  fpga_set_printmark(int window, int ignore)
{
	int pgNo;
	FpgaQSys->window_fifo  = window;
	FpgaQSys->ignored_fifo = ignore;
	for (pgNo=0; pgNo<SIZEOF(Fpga->cfg.pg); pgNo++) Fpga->cfg.pg[pgNo].fifos_ready = TRUE;
}

//--- _pg_ctrl ------------------------------------------------------
static void  _pg_ctrl(void)
{	
	if (_Socket!=INVALID_SOCKET)
	{
		int pg = Fpga->stat.encOut[0].PG_cnt;
		if (_PrintGo_Locked) pg=0;
		int  error = 	RX_EncoderStatus.fifoEmpty_PG  != Fpga->stat.pg_fifo_empty_err
					||	RX_EncoderStatus.fifoEmpty_IGN != Fpga->stat.ignored_fifo_empty_err
					||	RX_EncoderStatus.fifoEmpty_WND != Fpga->stat.window_fifo_empty_err;	

		RX_EncoderStatus.fifoEmpty_PG  = Fpga->stat.pg_fifo_empty_err;
		RX_EncoderStatus.fifoEmpty_IGN = Fpga->stat.ignored_fifo_empty_err;
		RX_EncoderStatus.fifoEmpty_WND = Fpga->stat.window_fifo_empty_err;	

		if (pg!=RX_EncoderStatus.PG_cnt || error)
		{
//			TrPrintfL(TRUE, "PrintGo %d", pg);
			RX_EncoderStatus.PG_cnt = pg;
			sok_send_2(&_Socket, REP_ENCODER_STAT, sizeof(RX_EncoderStatus), &RX_EncoderStatus);	
		}			
	}
}

//--- _corr_ctrl ------------------------------------------------------
static void  _corr_ctrl(void)
{
	static UINT32	_lastPos=0;
	static UINT32	_rest=0;
	
	if(Fpga->cfg.encIn[0].enable)
	{
		UINT32 pos = Fpga->stat.encOut[0].position;
		UINT32 len = (pos - _lastPos) % 0x100000;
		UINT32 m   = _rest + len*_StrokeDist;
		if (m > 1000000)
		{
			RX_EncoderStatus.meters += m/1000000;
			_rest = m%1000000;
			_lastPos = pos;
		}
	}
	else 
	{
		_lastPos = 0;
		_rest = 0;
	}
		
	if (Fpga->cfg.encIn[0].correction==CORR_ROTATIVE)
	{
		RX_EncoderStatus.corrRotPar[0] = Fpga->stat.encIn[0].ident_obs_a1;
		RX_EncoderStatus.corrRotPar[1] = Fpga->stat.encIn[0].ident_obs_b1;			
		
		//--- Vorschlag neue Werte (0,0)=kein neuer Wert ------------  
		RX_EncoderStatus.corrRotPar[2] = 0; // neuer Wert für pCfg->corrRotPar[0];
		RX_EncoderStatus.corrRotPar[3] = 0; // neuer Wert für pCfg->corrRotPar[1];	
	}
}

//--- fpga_is_init -----------------------------
int  fpga_is_init(void)
{
	return _Init;
}


//--- __level ------------------------------------------------------------------
static const char *_level(int level)
{
	static const char* _ON="*";
	static const char* _OFF="_";
	if (level) return _ON;
	else       return _OFF;
}

//--- _fpga_display_status -----------------------------------------------
static void _fpga_display_status(int showCorrection)
{
	int cnt=8;
	int i, n;

	if (_Init)
	{
		term_printf("\n");

		term_printf("--- FPGA Status ----------------------------------------\n");
		term_printf("Version: %d.%d.%d.%d\n",	Fpga->stat.version.major, Fpga->stat.version.minor, Fpga->stat.version.revision, Fpga->stat.version.build);

		{term_printf("\n");}
		{term_printf("input config     "); for (i=0; i<cnt; i++) term_printf("____%d____  ", i);								term_printf("\n");}
		{term_printf("  enable:        "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->cfg.encIn[i].enable);				term_printf("\n");}
		{term_printf("  orientation:   "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->cfg.encIn[i].orientation);			term_printf("\n");}
if(_ALL){term_printf("  index_on_b:    "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->cfg.encIn[i].index_on_b);			term_printf("\n");}
		{term_printf("  inc_per_rev:   "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->cfg.encIn[i].inc_per_revolution);	term_printf("\n");}
		{term_printf("  correction:    "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->cfg.encIn[i].correction);			term_printf("\n");}
		{term_printf("  corr_par:      "); for (i=0; i<cnt; i++) term_printf("%09d  ", FpgaCorr->par[i].par);					term_printf("\n");}
		{term_printf("  index_en:      "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->cfg.encIn[i].index_en);			term_printf("\n");}
if(_ALL){term_printf("  reset_pos:     "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->cfg.encIn[i].reset_pos);			term_printf("\n");}
				
		{term_printf("\n");}
		{term_printf("output config    "); for (i=0; i<cnt; i++) term_printf("____%d____  ", i);								term_printf("\n");}
		{term_printf("  encoder_no:    "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->cfg.encOut[i].encoder_no);			term_printf("\n");}
		{term_printf("  dist_ratio:    "); for (i=0; i<cnt; i++) term_printf("%09u  ", Fpga->cfg.encOut[i].dist_ratio);			term_printf("\n");}
		{term_printf("  synth_freq:    "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->cfg.encOut[i].synthetic_freq);		term_printf("\n");}
if(_ALL){term_printf("  backlash:      "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->cfg.encOut[i].backlash);			term_printf("\n");}
		{term_printf("  scanning:      "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->cfg.encOut[i].scanning);			term_printf("\n");}

		term_printf("\n");
		term_printf("PRINT GO config  "); term_printf("\n");
		term_printf("  enc_start_fwd: "); for (i=0; i<cnt; i++)term_printf("%09d  ", Fpga->cfg.pg[i].enc_start_pos_fwd);	term_printf("\n");
		term_printf("  enc_start_bwd: "); for (i=0; i<cnt; i++)term_printf("%09d  ", Fpga->cfg.pg[i].enc_start_pos_bwd);	term_printf("\n");
		term_printf("  pos_pg_fwd:    "); for (i=0; i<cnt; i++)term_printf("%09d  ", Fpga->cfg.pg[i].pos_pg_fwd);			term_printf("\n");
		term_printf("  pos_pg_bwd:    "); for (i=0; i<cnt; i++)term_printf("%09d  ", Fpga->cfg.pg[i].pos_pg_bwd);			term_printf("\n");
if(_ALL){term_printf("  printgo_n:     "); for (i=0; i<cnt; i++)term_printf("%09d  ", Fpga->cfg.pg[i].printgo_n);			term_printf("\n");}
		term_printf("  fifos_used:    "); for (i=0; i<cnt; i++)term_printf("%09d  ", Fpga->cfg.pg[i].fifos_used);			term_printf("\n");				
		term_printf("  fifos_ready:   "); for (i=0; i<cnt; i++)term_printf("%09d  ", Fpga->cfg.pg[i].fifos_ready);			term_printf("\n");
		term_printf("  dig_in_sel:    "); for (i=0; i<cnt; i++)term_printf("%09d  ", Fpga->cfg.pg[i].dig_in_sel);			term_printf("\n");
		term_printf("  quiet_window:  "); for (i=0; i<cnt; i++)term_printf("%09d  ", Fpga->cfg.pg[i].quiet_window);			term_printf("\n");
		term_printf("  PG Fifo Lvl:         %03d   DistTelCnt=%06d \n", FpgaQSys->printGo_status.fill_level, _DistTelCnt); 

		term_printf("\n");
		term_printf("input status     "); for (i=0; i<cnt; i++) term_printf("____%d____  ", i);								term_printf("\n");
		term_printf("  position:      "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encIn[i].position);		term_printf("\n");
//		term_printf("  StepTime:      "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encIn[i].step_time);		term_printf("\n");
//		term_printf("  StepTime Min:  "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encIn[i].setp_time_min);	term_printf("\n");
//		term_printf("  StepTime Max:  "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encIn[i].step_time_max);	term_printf("\n");
		term_printf("  rev_sum:       "); for (i = 0; i < cnt; i++) term_printf("%09d  ", Fpga->stat.encIn[i].rev_sum); term_printf("\n");
		term_printf("  inc_per_rev:   "); for (i = 0; i < cnt; i++) term_printf("%09d  ", Fpga->cfg.encIn[i].inc_per_revolution); term_printf("\n");
		term_printf("  rev_sum:       "); for (i = 0; i < cnt; i++) 
										{
											n=Fpga->cfg.encIn[i].inc_per_revolution;
											if (n) term_printf("%03d(%03d)   ", Fpga->stat.encIn[i].rev_sum, Fpga->stat.encIn[i].position / Fpga->cfg.encIn[i].inc_per_revolution); 
											else   term_printf("%03d(%03d)   ", Fpga->stat.encIn[i].rev_sum, 0); 											
										}
										term_printf("\n");
		term_printf("  i_to_a:        "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encIn[i].i_to_a);			term_printf("\n");
		term_printf("  i_to_b:        "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encIn[i].i_to_b);			term_printf("\n");
		term_printf("  setp_time_min: "); for (i=0; i<4; i++)   term_printf("%09d  ", Fpga->stat.encIn[i].setp_time_min);	term_printf("\n");
		
		term_printf("  lin corr diff: "); for (i = 0; i < cnt; i++) term_printf("%09d  ", (INT16)Fpga->stat.encIn[i].enc_diff); term_printf("\n");
		term_printf("  lin diff min:  "); for (i = 0; i < cnt; i++) term_printf("%09d  ", (INT16)Fpga->stat.encIn[i].enc_diff_min); term_printf("\n");
		term_printf("  lin diff max:  "); for (i = 0; i < cnt; i++) term_printf("%09d  ", (INT16)Fpga->stat.encIn[i].enc_diff_max); term_printf("\n");
		term_printf("  lin diff err:  "); for (i = 0; i < cnt; i++) term_printf("%09d  ", (INT16)Fpga->stat.encIn[i].enc_diff_overflow); term_printf("\n");

		term_printf("  rol_coeff:     "); for (i=0; i<4; i++)   term_printf("%09d  ", Fpga->stat.rol_coeff_at_use[i]);		term_printf("\n");
		term_printf("  identified b1: "); for (i = 0; i < cnt; i++) term_printf("%09d  ", Fpga->stat.encIn[i].ident_obs_b1); term_printf("\n");
		term_printf("  identified a1: "); for (i = 0; i < cnt; i++) term_printf("%09d  ", Fpga->stat.encIn[i].ident_obs_a1); term_printf("\n");
		/*
		term_printf("\n");
		term_printf("driver status    "); for (i = 0; i < 4; i++) term_printf("____%d____  ", i); term_printf("\n");
		term_printf("  driver a:      "); for (i = 0; i < 4; i++) term_printf("%09d  ", Fpga->stat.drive_err[i].chanel_a); term_printf("\n");
		term_printf("  driver b:      "); for (i = 0; i < 4; i++) term_printf("%09d  ", Fpga->stat.drive_err[i].chanel_b); term_printf("\n");
		term_printf("  driver i:      "); for (i = 0; i < 4; i++) term_printf("%09d  ", Fpga->stat.drive_err[i].chanel_i); term_printf("\n");
		term_printf("  driver d:      "); for (i = 0; i < 4; i++) term_printf("%09d  ", Fpga->stat.drive_err[i].chanel_d); term_printf("\n");
		*/
		
		term_printf("\n");
		term_printf("output status    "); for (i=0; i<cnt; i++) term_printf("____%d____  ", i);									term_printf("\n");
		term_printf("  position:      "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encOut[i].position);			term_printf("\n");
//		term_printf("  Speed:         "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encOut[i].speed*23/100);		term_printf("\n");
		term_printf("  Speed Min:     "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encOut[i].speed_min*23/100);	term_printf("\n");
		term_printf("  Speed Max:     "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encOut[i].speed_max*23/100);	term_printf("\n");
		term_printf("  PG Cnt:        "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encOut[i].PG_cnt);				term_printf("\n");
		term_printf("  PG wnd error:  "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.dig_pg_window_err[i]);			term_printf("\n");
//		term_printf("  mem_pointer:   "); for (i=0; i<3; i++)   term_printf("%09d  ", Fpga->stat.mem_pointer[i]);				term_printf("\n");

		term_printf("  FIFO Erors:    "); term_printf("PG:%06d  ",  Fpga->stat.pg_fifo_empty_err); 
										  term_printf("IGN:%05d  ", Fpga->stat.ignored_fifo_empty_err); 
										  term_printf("WND:%05d",   Fpga->stat.window_fifo_empty_err); 
										  term_printf("\n");	
		term_printf("  TCP/IP:        "); term_printf("CON:%05d  ", ctrl_connected()); 
										  term_printf("REQ:%05d  ", ctrl_requests()); 
										  term_printf("REP:%05d",   ctrl_replies()); 
										  term_printf("\n");
		
		if (showCorrection)
		{
			term_printf("\n");
			term_printf("--- CORR_ROTATIVE configuration -------------------------------------\n");
			term_printf("  Rol identification en:     %09d  ", Fpga->cfg.general.ident_rol_en);				term_printf("\n");
			term_printf("  Rol single sin en:         %09d  ", Fpga->cfg.general.single_sin_en);			term_printf("\n");
			term_printf("  Rol use internal ident en: %09d  ", Fpga->cfg.general.use_internal_ident_en);	term_printf("\n");
			term_printf("  Rol use new coeff en:      %09d  ", Fpga->cfg.general.runnung_avg_coeff);		term_printf("\n");	
			term_printf("  Rol swap rols en:          %09d  ", Fpga->cfg.general.rol_2_first);				term_printf("\n");
			term_printf("  Rol drift corr coeff:      %09d  ", Fpga->cfg.general.rol_drift_mu_two);			term_printf("\n");
			term_printf("  Rol  coeff b1:             %09d  ", Fpga->cfg.general.rol_0_new_b1);				term_printf("\n");
			term_printf("  Rol  coeff a1:             %09d  ", Fpga->cfg.general.rol_0_new_a1);				term_printf("\n");
			term_printf("  Rol2 coeff b1:             %09d  ", Fpga->cfg.general.rol_1_new_b1);				term_printf("\n");
			term_printf("  Rol2 coeff a1:             %09d  ", Fpga->cfg.general.rol_1_new_a1);				term_printf("\n");
		}	

		//--- calculate always ----
		int b1_0 = Fpga->stat.rol_coeff_at_use[0];
		int a1_0 = Fpga->stat.rol_coeff_at_use[1];
		int b1_1 = Fpga->stat.rol_coeff_at_use[2];
		int a1_1 = Fpga->stat.rol_coeff_at_use[3];
		int b1_0_ident = Fpga->stat.encIn[0].ident_obs_b1;
		int a1_0_ident = Fpga->stat.encIn[0].ident_obs_a1;
		double b1_0_old = b1_0 / 32768.0;
		double a1_0_old = a1_0 / 32768.0;
		double b1_0_new = b1_0_ident / 32768.0;
		double a1_0_new = a1_0_ident / 32768.0;
		double ampl_old = sqrt(b1_0_old*b1_0_old + a1_0_old*a1_0_old);
		double ampl_new = sqrt(b1_0_new*b1_0_new + a1_0_new*a1_0_new);

		if (Fpga->cfg.encIn[0].correction==CORR_ROTATIVE)
		{
			RX_EncoderStatus.ampl_old = (INT32)(10.0*ampl_old);
			RX_EncoderStatus.ampl_new = (INT32)(10.0*ampl_new);
			RX_EncoderStatus.percentage = (INT32)(10.0*(ampl_new * 100 / ampl_old - 100));	
		}
		//--- 
		
		if (showCorrection)
		{
			term_printf("\n");
			term_printf("--- CORR_ROTATIVE status -------------------------------------\n");
			term_printf("  Rol Corr Coeff b1:      %09f  ", b1_0_old);;
			term_printf("  Rol Corr Coeff 2 b1:    %09f  ", b1_0_new); term_printf("\n");
			term_printf("  Rol Corr Coeff a1:      %09f  ", a1_0_old);
			term_printf("  Rol Corr Coeff 2 a1:    %09f  ", a1_0_new); term_printf("\n");
			term_printf("  Rol Corr  0 peak in um: %09f  ", ampl_old);;
			term_printf("  Rol Ident 0 peak in um: %09f  ", ampl_new); term_printf("\n");
			term_printf("  Rol Ident improves Corr:%01f  ", 0.1 * RX_EncoderStatus.percentage); term_printf("% \n");
			term_printf("  Drift w coeff:          %09d  ", Fpga->stat.drift_w_coeff);		  term_printf("\n");
			term_printf("  Rol Corr pos 0:         %09d  ", Fpga->stat.rol_corr_pos_0);;	
			term_printf("  Rol Corr pos 1:         %09d  ", Fpga->stat.rol_corr_pos_1);	  term_printf("\n");
			term_printf("  Rol Corr flags 0:       0x%02x       ", Fpga->stat.rol_flags_0);;
			term_printf("  Rol Corr flags 1:       0x%02x  ", Fpga->stat.rol_flags_1);						term_printf("\n");
			term_printf("  Rol Corr rev sum long 0:%09d  ", Fpga->stat.rev_sums_long_0);;
			term_printf("  Rol Corr rev sum long 1:%09d  ", Fpga->stat.rev_sums_long_1);					term_printf("\n");
			term_printf("  Rol Corr stepsize 0:    %09d  ", Fpga->stat.rev_sums_long_0 * 128 / 32.0 / 238000.0); ;
			term_printf("  Rol Corr stepsize 1:    %09d  ", Fpga->stat.rev_sums_long_1 * 128 / 32.0 / 238000.0); term_printf("\n");
//			term_printf("  Rol Corr divfactor 0:   %09d  ", Fpga->stat.rev_sums_long_0 * 128 / 238000.0);;
//			term_printf("  Rol Corr divfactor 1:   %09d  ", Fpga->stat.rev_sums_long_1 * 128 / 238000.0); term_printf("\n");
			term_printf("  Rol Corr ramp 0:        %09d  ", Fpga->stat.ramp_value_0);;
			term_printf("  Rol Corr ramp 1:        %09d  ", Fpga->stat.ramp_value_1);						term_printf("\n");
			term_printf("  Rol Corr fill lvl 0:    %09d  ", (INT16)Fpga->stat.corr_out_fill_level_0);;			
			term_printf("  Rol Corr fill lvl 1:    %09d  ", (INT16)Fpga->stat.corr_out_fill_level_1);		term_printf("\n");
			term_printf("  Rol Corr delays busy 0: %09d  ", Fpga->stat.corr_out_delays_busy_0);;			
			term_printf("  Rol Corr delays busy 1: %09d  ", Fpga->stat.corr_out_delays_busy_1);			term_printf("\n");
			term_printf("  Rol Corr delays err 0:  %09d  ", Fpga->stat.corr_out_delays_err_0);;			
			term_printf("  Rol Corr delays err 1:  %09d  ", Fpga->stat.corr_out_delays_err_1);				term_printf("\n");
			term_printf("  Curr ratio 0:           %09f  ", (double)Fpga->stat.curr_ratio_0 / 0x80000000);;			
			term_printf("  Curr ratio 1:           %09f  ", (double)Fpga->stat.curr_ratio_1 / 0x80000000); term_printf("\n");	
			term_printf("  Rol Corr fill lvl 0 max:%09d  ", (INT16)Fpga->stat.corr_out_fill_level_0_max); term_printf("\n");			
			term_printf("  Rol Corr fill lvl 1 min:%09d  ", (INT16)Fpga->stat.corr_out_fill_level_0_min); term_printf("\n");	
			term_printf("  Err Vec 0:              %09x  ", (INT16)Fpga->stat.rolcor_0_err_vec);;			
			term_printf("  Err Vec 1:              %09x  ", (INT16)Fpga->stat.rolcor_1_err_vec); term_printf("\n");
			term_printf("  Delays busy 0 max:      %09d  ", Fpga->stat.rolcor_0_delay_busy_max); term_printf("\n");
			term_printf("  identified b1: "); for (i = 0; i < 2; i++) term_printf("%09d  ", Fpga->stat.encIn[i].ident_obs_b1); term_printf("\n");
			term_printf("  identified a1: "); for (i = 0; i < 2; i++) term_printf("%09d  ", Fpga->stat.encIn[i].ident_obs_a1); term_printf("\n");
			term_printf("  rev_sum:       "); for (i = 0; i < 2; i++) term_printf("%09d  ", Fpga->stat.encIn[i].rev_sum); term_printf("\n");
		}
				
		term_printf("\n");
		if (FALSE && RX_EncoderCfg.scanning)
		{
			term_printf("  scans:  %06d  pg: %06d(%06d) left:   %06d    right:   %06d \n", _scanpos_cnt, Fpga->stat.encOut[0].PG_cnt, _scanpos_pg, _scanpos_left, _scanpos_right);				
		}
		/*
		term_printf("\n");
		term_printf("  in_pulse_cnt:  %06d    stroke_enc_pulse_cnt:   %06d\n", Fpga->stat.in_pulse_cnt,  Fpga->stat.stroke_enc_pulse_cnt);
		term_printf("  out_pulse_cnt: %06d    stroke_enc_end_pos:     %06d\n", Fpga->stat.out_pulse_cnt, Fpga->stat.stroke_enc_end_pos);
		term_printf("  in_end_pos:    %06d    stroke_sum_end_pos:     %06d\n", Fpga->stat.in_end_pos,	 Fpga->stat.stroke_sum_end_pos);
		term_printf("  out_end_pos:   %06d    \n", Fpga->stat.out_end_pos);
		term_printf("  dir_change_cnt:        "); for (i=0; i<2; i++) term_printf("%09d  ", Fpga->stat.dir_change_cnt[i]);	term_printf("\n");
		term_printf("  ab_change_error:       "); for (i=0; i<4; i++) term_printf("%09d  ", Fpga->stat.ab_change_error[i]);	term_printf("\n");
				
		term_printf("\n");
		term_printf("common status    FPGA	ENCODER\n");
		term_printf("  info:          0x%08x  0x%08x\n", Fpga->stat.info,  RX_EncoderStatus.info);
		term_printf("  warn:          0x%08x  0x%08x\n", 0,				   RX_EncoderStatus.warn);
		term_printf("  error:         0x%08x  0x%08x\n", Fpga->stat.error, RX_EncoderStatus.err);
		term_printf("  enc_alarm:     0x%01x         0x%06x\n", FpgaQSys->pio_enc_alarm_d_in, FpgaQSys->pio_enc_alarm_in);
		*/
	//	term_printf("  statistics:    "); for (i=0; i<cnt; i++) term_printf("0x%08x ", Fpga->stat.statistics[i]);			term_printf("\n");
	//	term_printf("  msg cnt:       %06d\n", Fpga->stat.msg_cnt);

		if(FALSE)
		{
			term_printf("\n");
			term_printf("UV Lamp  --------------------------------------\n");
			term_printf("  speed: %d\n", _UV_Speed);

			term_printf("- Output ---------------- Input ---------------------------\n"); 
			term_printf("  00: UV-Insulator: %s     00: UV-Ready: %s\n", _level(FpgaQSys->out&0x0001), _level(FpgaQSys->in&0x0001));
			term_printf("  01: UV-Power:     %s     01:         : %s\n", _level(FpgaQSys->out&0x0002), _level(FpgaQSys->in&0x0002));
			term_printf("  02: UV-Shutter:   %s     02: UV-Fault: %s\n", _level(FpgaQSys->out&0x0004), _level(FpgaQSys->in&0x0004));
			term_printf("  03:               %s     03:           %s\n", _level(FpgaQSys->out&0x0008), _level(FpgaQSys->in&0x0008));				
		}

		/*
		term_printf("\n");
		term_printf("FIFOs  -------- PG -- Wnd - Ign -----------------------------\n");
		term_printf("fill level:      %03d   %03d   %03d\n",	FpgaQSys->printGo_status.fill_level		 , FpgaQSys->window_status.fill_level			, FpgaQSys->ignored_status.fill_level			);
		term_printf("i status:       0x%02x  0x%02x  0x%02x\n",	FpgaQSys->printGo_status.i_status		 , FpgaQSys->window_status.i_status			, FpgaQSys->ignored_status.i_status			);
		term_printf("event:          0x%02x  0x%02x  0x%02x\n",	FpgaQSys->printGo_status.event			 , FpgaQSys->window_status.event				, FpgaQSys->ignored_status.event				);
		term_printf("int enable:     0x%02x  0x%02x  0x%02x\n",	FpgaQSys->printGo_status.interrupt_enable, FpgaQSys->window_status.interrupt_enable	, FpgaQSys->ignored_status.interrupt_enable	);
		term_printf("almost full:     %03d   %03d   %03d\n",	FpgaQSys->printGo_status.almost_full     , FpgaQSys->window_status.almost_full		, FpgaQSys->ignored_status.almost_full		);
		term_printf("almost empty:    %03d   %03d   %03d\n",	FpgaQSys->printGo_status.almost_empty	 , FpgaQSys->window_status.almost_empty		, FpgaQSys->ignored_status.almost_empty		);	
		*/
				
//		test_do();
//		if (Fpga->stat.encOut[0].PG_cnt)
//			term_flush();
	}
}

//--- _fpga_display_status_test -----------------------------------------------
static void _fpga_display_status_test(void)
{
	int cnt=4;
	int i;

	if (_Init)
	{
		term_printf("\n");

		term_printf("--- FPGA Status TEST -----------------------------------\n");
		term_printf("Version: %d.%d.%d.%d\n",	Fpga->stat.version.major, Fpga->stat.version.minor, Fpga->stat.version.revision, Fpga->stat.version.build);

		term_printf("\n");
		term_printf("input status    "); for (i=0; i<cnt; i++) term_printf("____%d____  ", i);								term_printf("\n");
		term_printf("  rev_sum:      "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encIn[i].rev_sum);			term_printf("\n");
		term_printf("  i_to_a:       "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encIn[i].i_to_a);			term_printf("\n");
		term_printf("  i_to_b:       "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encIn[i].i_to_b);			term_printf("\n");
		term_printf("  position:     "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encIn[i].position);			term_printf("\n");
		term_printf("  StepTime:     "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encIn[i].step_time);		term_printf("\n");
		term_printf("  StepTime Min: "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encIn[i].setp_time_min);	term_printf("\n");
		term_printf("  StepTime Max: "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encIn[i].step_time_max);	term_printf("\n");
		term_printf("  Input:        "); for (i=0; i<cnt; i++) {if (FpgaQSys->in &(1<<i)) term_printf("* "); else term_printf("_ ");};	term_printf("\n");
		term_printf("  Output:       "); for (i=0; i<cnt; i++) {if (FpgaQSys->out&(1<<i)) term_printf("* "); else term_printf("_ ");};	term_printf("\n");
		term_printf("  Output Status:"); for (i=0; i<cnt; i++) {if (FpgaQSys->out_status&(1<<i)) term_printf("* "); else term_printf("_ ");};	term_printf("\n");
		term_printf("\n");
		term_printf("output status   "); for (i=0; i<cnt; i++) term_printf("____%d____  ", i);								term_printf("\n");
		term_printf("  position:     "); for (i=0; i<cnt; i++) term_printf("%09d  ", Fpga->stat.encOut[i].position);		term_printf("\n");
	}
}

//--- _fpga_display_error --------------------------------------------------
static void _fpga_display_error(void)
{
	if (_Init)
	{
	}
}

//--- _check_errors ---------------------------------------------------------------------
static void  _check_errors(void)
{	
	if (_Scanning)	return;
	
	//--- check for index -------------------------------------------------------------
	#define CHECK_POS	1000000
	#define INC_PER_REV	238000
	static int _pos[8];
	int			i;
//	for (int i=0; i<SIZEOF(Fpga->cfg.encIn); i++)
	for (i=0; i<2; i++)
	{
		if(Fpga->cfg.encIn[i].enable && Fpga->cfg.encIn[i].index_en && !_IndexCheckDone)
		{
			if (_pos[i]<CHECK_POS)
			{
				_pos[i] = Fpga->stat.encIn[i].position;
				if (_pos[i] >= CHECK_POS)
				{
					_IndexCheckDone = TRUE;
					if (Fpga->stat.encIn[i].rev_sum==0)						Error(ERR_CONT, 0, "Encoder %d: No Index", i);
					if (Fpga->cfg.encIn[0].inc_per_revolution!=INC_PER_REV) Error(ERR_CONT, 0, "Encoder %d: Counting %d increments per revolution (expected %d)", i, Fpga->cfg.encIn[0].inc_per_revolution, INC_PER_REV);
				}
			}
		}
		else _pos[i]=0;
	}
}

//--- _scan_check_init --------------------------------------
static void _scan_check_init(void)
{
	_scanpos_last  = 0;
	_scanpos_left  = 0;
	_scanpos_right = 0;
	_scanpos_cnt   = 0;
	_scanpos_pg    = 0;
	_PG_cnt		   = 0;
	_scanpos_fwd   = TRUE;	
}
	
//--- _scan_check ----------------------------------------
static void	 _scan_check(void)
{
	if (RX_EncoderCfg.scanning)
	{
		UINT32 pos = Fpga->stat.encIn[0].position;

		if (_scanpos_left==0  || pos<_scanpos_left ) _scanpos_left  = pos;		
		if (_scanpos_right==0 || pos>_scanpos_right) _scanpos_right = pos;
				
		if (_scanpos_fwd)
		{
			if (pos+1000<_scanpos_right && pos+1000>Fpga->cfg.pg[0].enc_start_pos_fwd)
			{
				_scanpos_cnt++;
				_scanpos_fwd  = FALSE;			
				_scanpos_left = 0;
				_scanpos_pg++;
			}
		}
		else
		{
			if (pos < Fpga->cfg.pg[0].enc_start_pos_fwd) 
			{
				if (_scanpos_right>Fpga->cfg.pg[0].enc_start_pos_bwd) _scanpos_pg++;
				_scanpos_cnt++;
				_scanpos_fwd   = TRUE;
				_scanpos_right = 0;
			}		
		}
		
		while ((Fpga->stat.encOut[0].PG_cnt & 0xff) != (_PG_cnt & 0xff)) 
		{
			_PG_cnt++;
		}

		if (_PG_cnt<_scanpos_pg) 
			ErrorFlag(ERR_ABORT, &_ErrFlags, 1<<0, 0, "PrintGo Missed (expected=%d, sent=%d)", _scanpos_pg, Fpga->stat.encOut[0].PG_cnt);
		_scanpos_last = pos;
	}
}

