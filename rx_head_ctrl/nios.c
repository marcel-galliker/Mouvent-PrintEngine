// ****************************************************************************
//
//	DIGITAL PRINTING - nios.c
//
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#ifdef soc
	#include <sys/mman.h>
	#include <stdio.h>
	#include <math.h>
#endif
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_fpga.h"
#include "rx_term.h"
#include "rx_trace.h"
#include "fpga_def_head.h"
#include "hex2bin.h"
#include "args.h"
#include "firepulse.h"
#include "fpga.h"
#include "nios.h"
#include "tse.h"
#include "EEprom.h"
#include "rx_threads.h"
#include "rx_head_ctrl.h"
#include "putty.h"
#include "version.h"
#include "conditioner.h"

#define	NIOS_EXE_ADDR	0xc0000000
#define NIOS_EXE_SIZE	0x00010000

#define NIOS_MEM_ADDR	0xc0060000
#define NIOS_MEM_SIZE	(20*1024)   //0x00010000

#define NIOS_RESET_ADDR	0xFF220000	// 0xFF220010
#define NIOS_RESET_SIZE	0x00001000

//--- external variables (also used in conditioner) -----
SNiosCfg			*_NiosCfg;	
SNiosStat			*_NiosStat;
SNiosMemory			*_NiosMem;

//--- module globals ---------------------------------------
static int _NiosInit   = FALSE;
static int _NiosLoaded = FALSE;

static UINT8			_GreyLevel[MAX_HEADS_BOARD][MAX_DROP_SIZES];
static int				_DotSize1;
static UINT32			_FPWarning;
static int				_CoolerErr;
int						NIOS_FixedDropSize=0;
int						NIOS_Droplets=0;
static int				_NiosReady=FALSE;
static int				_MaxSpeed;

//--- prototypes -----------------------------------

static void _nios_copy_status(void);
static void _nios_set_user_eeprom(int no);

//#define FASTLOG
#ifdef FASTLOG	
	static FILE	*_FastLog_File = NULL;
	static void _nios_fastlog_start(void);
	static void _nios_fastlog(void);
#endif

//--- nios_shutdown --------------------------------
void nios_shutdown(void)
{
	if (rx_fpga_running())
	{
		int fd = open("/dev/mem", O_RDWR); 
		_NiosMem  = (SNiosMemory*)mmap(NULL, NIOS_MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, NIOS_MEM_ADDR);
		_NiosCfg  = &_NiosMem->cfg;
		cond_ctrlMode2(-1, ctrl_off);
		close(fd);		
		nios_end();		
	}
}

//--- nios_init ---------------------------------
int nios_init(void)
{
	int tio;
	_NiosInit = TRUE;

	nios_load(PATH_BIN_HEAD FIELNAME_HEAD_NIOS);
	
	_NiosMem = NULL;
	_FPWarning = 0;
	_CoolerErr = 0;
	_DotSize1 = 0;
	memset(_GreyLevel, 0, sizeof(_GreyLevel));

	#ifdef soc
	if (_NiosLoaded)
	{
		int fd = open("/dev/mem", O_RDWR); 
		_NiosMem  = (SNiosMemory*)mmap(NULL, NIOS_MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, NIOS_MEM_ADDR);
		close(fd);
	}
	#endif
	
	if (_NiosMem==NULL) 
	{
		_NiosMem = (SNiosMemory*)rx_malloc(sizeof(SNiosMemory));
		_NiosMem->stat.info.is_shutdown = TRUE;	
	}
	if (sizeof(SNiosMemory)>NIOS_MEM_SIZE) return Error(ERR_ABORT, 0, "NIOS DP-Memory too small. (NIOS_MEM_SIZE=%d, SNiosMemory.size=%d)", NIOS_MEM_SIZE, sizeof(SNiosMemory));
	
	_NiosCfg  = &_NiosMem->cfg;
	_NiosStat = &_NiosMem->stat;
	
	_NiosMem->cfg.cmd.debug = arg_debug;

	//--- wait until nios ready ----------------------
	for (tio=0; !_NiosMem->stat.info.nios_ready && tio<10; tio++) 
	{
		rx_sleep(100);
	}
	if (!_NiosMem->stat.info.nios_ready)
	{
		TrPrintfL(TRUE,"NIOS NOT READY");
		Error(WARN, 0, "NIOS NOT READY");
	}
	
	TrPrintfL(TRUE, "Nios Version = %d.%d.%d.%d", _NiosStat->version.major, _NiosStat->version.minor, _NiosStat->version.revision, _NiosStat->version.build);
	RX_HBStatus[0].niosVersion.major	= _NiosStat->version.major;
	RX_HBStatus[0].niosVersion.minor	= _NiosStat->version.minor;
	RX_HBStatus[0].niosVersion.revision	= _NiosStat->version.revision;
	RX_HBStatus[0].niosVersion.build	= _NiosStat->version.build;
	
#ifdef FASTLOG	
	_nios_fastlog_start();
#endif
	return REPLY_OK;
}

//--- nios_end -------------------------------------
int nios_end(void)
{
	if (_NiosLoaded) 
	{
		// required to not damage Print Heads !
		//	putty_end();
		cond_end();
		fpga_master_disable();
		rx_sleep(200);
		nios_set_firepulse_on(FALSE);
		_NiosCfg->cmd.shutdown = TRUE;
		rx_sleep(1000);
		_NiosInit = FALSE;

		#ifdef soc
			munmap(_NiosMem, NIOS_MEM_SIZE); 
		#endif
		_NiosMem = NULL;
		_NiosLoaded = FALSE;			
	}
	return REPLY_OK;
}

//--- nios_loaded ------------------------------------
int  nios_loaded(void)
{
	return _NiosLoaded;
}

//--- nios_load --------------------------------------
void nios_load(const char *exepath)
{
	TrPrintfL(TRUE, "nios_load >>%s<< ***", exepath);
	_NiosReady = FALSE;
	if (!fpga_is_init()) return;

#ifdef soc
	int		fd;
	INT32	size, read, tio;
	UINT32	*exeMem;
	UINT32  data;
	BYTE	buffer[0x10000];

	hex2bin(exepath, buffer, sizeof(buffer), &size);
	if (size)
	{
		if (_NiosMem)
		{
			_NiosMem->cfg.cmd.shutdown = TRUE;
			tio=0;
			while (!_NiosMem->stat.info.is_shutdown)
			{
				if (++tio>200)
				{
					Error(WARN, 0, "nios_load: Timeout on waiting for nios-shutdown");
					break;
	//				return;
				}

				rx_sleep(10);
			}
		}

		if (fpga_nios_reset(TRUE)!=REPLY_OK) return;

		if (_NiosMem) _NiosMem->cfg.cmd.shutdown = FALSE;

		/* Map Physical address of NIOS RAM to virtual address segment with Read/Write Access */ 
		fd = open("/dev/mem", O_RDWR);
		exeMem  = (UINT32*)mmap(NULL, NIOS_EXE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, NIOS_EXE_ADDR);
		memcpy(exeMem, buffer, size);
		mprotect(exeMem, NIOS_EXE_SIZE, PROT_READ);  
		munmap(exeMem,  NIOS_EXE_SIZE); 
		close(fd);
	
		if (arg_offline)
		{
			printf("\nPress <ENTER> when NIOS loaded.\n");
			getchar();
		}

		fpga_nios_reset(FALSE);
		_NiosLoaded = TRUE;
	}
#endif
}

//--- _sample_wf ----------------------------------------

#define OLD_WF_OFFSET		-20
#define OLD_ON_OFFSET		  0 // 0.6 �s
#define OLD_OFF_OFFSET		 48 // 0.6 �s
#define OLD_ALL_ON_OFFSET	 -50	// ALL-ON has to be set 50 cycles before last sub-pulse starts
#define FP_VALUE_30			1564
#define FP_VALUE_36			1820	//for test only
#define FP_VALUE_44			1920	//for test only, R6 = 430 Ohm

static void _sample_wf(int head, SInkDefinition *pink, char *dots, int fpVoltage, SFpgaHeadCfg *cfg)
{
	//--- cutting droplets -------------------
	// the droplet waveform starts with the trailing edge and ends with the trailing edge of the next droplet
	
#define MAX_TIME_WF 8000
	SWfPoint pts[MAX_WF_POINTS];
	int i, n, pos, mhz;
	int shift;			// to add leading zeros
	int cut;			// to remove sub pulses (length of removed sub pulses)	
	int allOnDroplet;
	int startAllOn=0;	// position of the "all on" subpulse
	int v0, v1;
	int p0, p1;
	int subPulsCnt;
	int voltageCnt;
	int dropletNo;
	int trace=FALSE;
	int load;
	int ret;
	int levels;
	int voltageRef;
	int valueRef;
	int lastOffPos;
	int first;
	int subPulses=1;
	int	wf_first_pulse_pos = WF_FIRST_PULSE_POS;
	int	wf_offset	  = OLD_WF_OFFSET;
	int on_offset	  = OLD_ON_OFFSET;
	int off_offset	  = OLD_OFF_OFFSET;
	int all_on_offset = OLD_ALL_ON_OFFSET;
	
	UINT16	voltage[VOLTAGE_SIZE];

	if (!_NiosLoaded) return;
	
	fpga_enc_enable(FALSE);
//	if (!strcmp(pink->fileName, _InkDef[head].fileName) && (subPulses==_MaxDropSize[head]) && (fpVoltage==_FpVoltage[head])) return
	
	if (fpVoltage && fpVoltage<30) ErrorFlag(WARN, &_FPWarning, 1<<head, 0, "Head[%d]: Firepulse Voltage=%d%, head may not print", head, fpVoltage);
		
	memcpy(&_GreyLevel[head], pink->greyLevel, sizeof(_GreyLevel[head]));

	{
		int s;
		char *ch;
		for (ch=dots, subPulses=0; *ch; ch++)
		{
			s=0;
			if(*ch=='S')	s=1;
			if(*ch=='M')	s=2;
			if(*ch=='L')	s=3;
			if (s>subPulses) subPulses=s;
		}
		if (subPulses==0) subPulses=3;
		NIOS_Droplets = subPulses;
		if(dots[1] == 0) _DotSize1 = s;
		else _DotSize1=0;
	}
	
	if (RX_HBStatus[0].fpgaVersion.build>=1737)
	{
		wf_first_pulse_pos = 0;			
		wf_offset		   = 0;
		on_offset		   = 0;
		off_offset		   = 58;
		all_on_offset	   = -9;
	}
	
	mhz=fpga_fp_clock_mhz();
	for (i=0; i<MAX_WF_POINTS; i++)
	{
		pts[i].pos  = ((UINT32)pink->wf[i].pos*mhz)/160;
		pts[i].volt = pink->wf[i].volt;		
	}
		
	TrPrintfL(trace, "");
	TrPrintfL(trace, "wf_first_pulse_pos = %d", wf_first_pulse_pos);
	TrPrintfL(trace, "wf_offset       = %d", wf_offset);
	TrPrintfL(trace, "on_offset       = %d", on_offset);
	TrPrintfL(trace, "off_offset      = %d", off_offset);
	TrPrintfL(trace, "all_on_offset   = %d", all_on_offset);
	TrPrintfL(trace, "");

	memset(voltage, 0, sizeof(voltage));
	levels = pink->greyLevel[subPulses];
	allOnDroplet = 0;
	for (i=0; i<8; i++)
	{
		if (pink->greyLevel[0] & (1<<i)) allOnDroplet=i;
	}
	for (i=0; i<SIZEOF(cfg->fp); i++) cfg->fp[i].on = cfg->fp[i].off = 0;
	
	pos = 0;
	p0 = 0;
	v0 = 0;
	shift=0;
	cut=0;
	voltageCnt = 0;
	subPulsCnt = 0;
	dropletNo  = 0;
	lastOffPos = 0;
	first	   = 0;
	
	//default values ----
	{
		voltageRef = 300; // 30 Volt
		valueRef   = FP_VALUE_30;			
	}
		
	/*
	if (48V)
	{
		// 48V puwer supply: Reference at 44V, 
		voltageRef = 440;
		valueRef   = FP_VALUE_44;			
	}

	if (36V)
	{
		// 36V puwer supply: Reference at 44V, 
		voltageRef = 360;
		valueRef   = FP_VALUE_36;			
	}
	*/
	
	if (_NiosMem->cfg.cond[head].mode==ctrl_print || (_NiosMem->cfg.cond[head].mode >= ctrl_recovery_start && _NiosMem->cfg.cond[head].mode <= ctrl_recovery_step4)) 
	{
		if (fpVoltage) 	valueRef = (valueRef*fpVoltage)/100;
		else			valueRef = valueRef;
	//	Error(LOG, 0, "Firepulse[%s].Voltage = %d%%", RX_HBConfig.head[head].name, fpVoltage);
	}
	else valueRef = 0;
		
	for (i=0; i<MAX_WF_POINTS; i++)
	{
		p1 = pts[i].pos-first+shift-cut;
		v1 = pts[i].volt*10;	// calculate in 1/10V to avoid rounding problems

		if (v0==0 && v1>0)
		{
			dropletNo++;
			if (subPulsCnt>=allOnDroplet)
			{
				if (!startAllOn) startAllOn = p0;
				cut = p0-startAllOn;
			}
			if (subPulsCnt==0)
			{
				while (pos<wf_first_pulse_pos)
				{
					voltage[pos++]=0;
					p0++;
					p1++;
					shift++;
				}
				if (shift) TrPrintfL(trace, "wf[%d]: p=%d, pcorr=%d, v=%d --- corrected", i-1, pts[i-1].pos, pts[i-1].pos+shift, pts[i-1].volt);
			}
			if (startAllOn)
			{				
				p0 = pos = startAllOn;
				p1 = startAllOn + (pts[i].pos-pts[i-1].pos);
			}
			cfg->fp[subPulsCnt].on = pos;
			cfg->fp_allOn		   = pos+all_on_offset; // All on for the last sub-pulse!
		}
		else if (v0!=0 && v1==0 && (levels & (0x01<<(dropletNo-1))))
		{
			cfg->fp[subPulsCnt].off = lastOffPos = p1+off_offset;
			if (subPulsCnt<subPulses && !startAllOn)
				subPulsCnt++;
			voltageCnt = p1+1+wf_offset;
		}
		TrPrintfL(trace, "wf[%d]: p=%d, pcorr=%d, v=%d", i, pts[i].pos, pts[i].pos+shift, pts[i].volt);
		if (p1<p0) break;
		if (p1!=p0)
		{
			if(v0==0 && v1==0 && subPulsCnt==0 && !first)
			{
				first = p1+1;
				p1=0;
			}
			else
			{
				if (!dropletNo || (levels & (0x01<<(dropletNo-1))))
				{
					for (n=0; pos<=p1; n++, pos++)
					{
						if (pos+wf_offset>0) voltage[pos+wf_offset] = ((v0 + (v1-v0) * (pos-p0)/(p1-p0)) * valueRef) / voltageRef;					
					}
				}	
				else
				{
					cut += p1-pos;
					p1=p0;
				}				
			}
		}
		p0 = p1;
		v0 = v1;
	}

	TrPrintfL(trace, "");
	//--- fill waveform ----
	TrPrintfL(trace, "Adding %d Fillers", lastOffPos-voltageCnt);
	while (voltageCnt<lastOffPos)
		voltage[voltageCnt++] = 0;
			
	//--- set FPGA values -------------------------
	cfg->fp_length	  = voltageCnt;
	cfg->fp_subPulses = subPulsCnt;
	double clock=12.500*160/140;
	double khz=1000000/(voltageCnt*clock);
	double mmin = khz*25.4/1200.0*60.0;
//	TrPrintfL(TRUE, "Head[%d]: dots: >>%s<< sub-pulses: %d, seq-length: %d, max Speed; %d m/min", head, dots, cfg->fp_subPulses, cfg->fp_length, (int)mmin);
	TrPrintfL(TRUE, "Head[%d]: dots: >>%s<<, max Speed=%d m/min", head, dots, (int)mmin);
	if (RX_HBConfig.ctrlAddr == sok_addr_32("192.168.200.11") && head==0) 
		Error(LOG, 0, "Head[%d]: dots: >>%s<<, max Speed=%d m/min", head, dots, (int)mmin);
	
	if (mmin>_MaxSpeed) _MaxSpeed = mmin;
	
	nios_fixed_grey_levels(0, subPulses);
	
	if (TRUE || trace)
	{
		for (i=0; i<4; i++)
		{
			TrPrintfL(TRUE, "gl[%d]=0x%02x", i, cfg->gl_2_pulse[i]);
		}
		TrPrintfL(TRUE, "");
		for (i=0; i<SIZEOF(cfg->fp); i++)
		{
			TrPrintfL(TRUE, "firepulse[%d]: on=%lu, off=%lu", i, cfg->fp[i].on, cfg->fp[i].off);
		}
		TrPrintfL(TRUE, "AllOn: %d", cfg->fp_allOn);

		if (FALSE)
		{
			TrPrintfL(TRUE, "Waveform Sample");
			for (i=0; i<voltageCnt; i++) TrPrintfL(TRUE, "wf[%d].volt=%d", i, voltage[i]);
			TrPrintfL(TRUE, "");
		}
		TrPrintfL(TRUE, "");
	}
	
	fp_set_waveform(head, voltageCnt, voltage);
	fpga_enc_enable(TRUE);
}

//--- nios_fixed_grey_levels ----------------------------
void nios_fixed_grey_levels(int fixedDropSize, int maxDropSize)
{
	int head, i, n, level;
	int droplet;
	if (fixedDropSize>3) fixedDropSize=3;
	if (fixedDropSize<0) fixedDropSize=0;
	if (maxDropSize>3)   maxDropSize=3;
	if (maxDropSize<1)	 maxDropSize=1;
	NIOS_FixedDropSize = fixedDropSize;
	for (head=0; head<SIZEOF(FpgaCfg.head); head++)
	{
		SFpgaHeadCfg *cfg = FpgaCfg.head[head];
		if (fixedDropSize == 0)
		{	
			for (i=0; i<SIZEOF(cfg->gl_2_pulse); i++)
			{
				for (n=0, level=0, droplet=0; n<8; n++)
				{
					if (_GreyLevel[head][maxDropSize] & (1<<n)) 
					{
						if (_GreyLevel[head][i] & (1<<n)) level |= (0x01<<droplet);
						droplet++;
					}
				}
				cfg->gl_2_pulse[i] = level;					
			}
			if (_DotSize1>1 && _DotSize1<=maxDropSize) cfg->gl_2_pulse[1] = cfg->gl_2_pulse[_DotSize1];
		}
		else
		{
			Error(WARN, 0, "TEST Fix Greylevel");		
			cfg->gl_2_pulse[0] = _GreyLevel[head][fixedDropSize];			
		}
	}
}

//--- nios_setInk ---------------------------------------------
void nios_setInk(int no, SInkDefinition *pink, char *dots, int fpVoltage)
{
	if (no==0) _MaxSpeed=0;
	if (no>=0 && no<HEAD_CNT)
	{
		_sample_wf(no, pink, dots, fpVoltage, FpgaCfg.head[no]);
		cond_setInk(no, pink);
		RX_RGB[no] = pink->colorRGB;
	}
//	if (no==HEAD_CNT-1) Error(LOG, 0, "MaxSpeed: %d m/min, Dotsize: %s", _MaxSpeed, dots);
}

//--- nios_set_firepulse_on ----------------------------------
void nios_set_firepulse_on(int on)
{
	if (_NiosMem && _NiosMem->cfg.cmd.firepulse_on!=on)
	{
		TrPrintfL(TRUE, "nios_set_firepulse_on(%d)", on);	
		_NiosMem->cfg.cmd.firepulse_on = on;
		_NiosMem->cfg.cmd.debug = arg_debug;
	}
}

//--- nios_is_firepulse_on ---------------------
int  nios_is_firepulse_on(void)
{	
	if (_NiosMem && _NiosMem->stat.powerState==power_all_on) return TRUE;
	return FALSE;				
}

//--- nios_main --------------------------------
int  nios_main(int ticks, int menu)
{	
#ifdef FASTLOG	
	_nios_fastlog();
#endif
	
	if (_NiosLoaded)
	{
		if (_NiosMem && _NiosStat) 
		{
			cond_main(ticks, menu);
            eeprom_main(ticks, menu);
		}
		tse_check_errors(ticks, menu);
		if (menu)
		{
			_nios_copy_status();			
			nios_check_errors(ticks);		
		}
	}
	return REPLY_OK;
}

//--- _nios_copy_status -----------------------------------
static void _nios_copy_status(void)
{
	memcpy(&RX_NiosStat, _NiosStat, sizeof(RX_NiosStat));
	RX_HBStatus->flow = RX_NiosStat.cooler_pressure;
	for (int head=0; head < MAX_HEADS_BOARD; head++)
	{
		RX_HBStatus->head[head].tempHead = RX_NiosStat.head_temp[head];
		if (RX_HBStatus->head[head].eeprom.serialNo==0 || RX_HBStatus->head[head].eeprom.serialNo==INVALID_VALUE) 
			eeprom_init_data(head, RX_NiosStat.eeprom_fuji[head], &RX_HBStatus->head[head].eeprom);
	}
}

//--- nios_check_errors ---------------------------
void nios_check_errors(int ticks)
{	
	ELogItemType	abort = RX_HBConfig.simuPlc ? LOG_TYPE_WARN : LOG_TYPE_ERROR_ABORT;

	if (fpga_is_ready())
	{
//		if (_NiosStat->error.fpga_incompatible)		ErrorFlag(ERR(abort), (UINT32*)&RX_HBStatus[0].err, err_nios_incompatible, 0, "Conditioner NIOS NOT compatible");
		if (_NiosStat->error.fpga_incompatible)		ErrorFlag(WARN, (UINT32*)&RX_HBStatus[0].err, err_nios_incompatible, 0, "Conditioner NIOS NOT compatible");
		if (_NiosStat->error.u_plus_2v5)			ErrorFlag(ERR(abort), (UINT32*)&RX_HBStatus[0].err, err_2_5volt, 0, "2.5Volt Power Supply Error");
		if (_NiosStat->error.u_plus_3v3)			ErrorFlag(ERR(abort), (UINT32*)&RX_HBStatus[0].err, err_3_3_volt, 0, "3.3Volt Power Supply Error");
		if (_NiosStat->error.u_plus_5v)				ErrorFlag(ERR(abort), (UINT32*)&RX_HBStatus[0].err, err_5volt, 0, "5Volt Power Supply Error");
		if (_NiosStat->error.u_minus_5v)			ErrorFlag(ERR(abort), (UINT32*)&RX_HBStatus[0].err, err_min_5volt, 0, "-5Volt Power Supply Error");
		if (_NiosStat->error.u_firepulse)			
		{
			if(_NiosStat->info.u_firepulse_48V)
				ErrorFlag(ERR(abort), (UINT32*)&RX_HBStatus[0].err, err_36volt, 0, "-48Volt Power Supply Error");				
			else
				ErrorFlag(ERR(abort), (UINT32*)&RX_HBStatus[0].err, err_36volt, 0, "-36Volt Power Supply Error");				
		}
		if (_NiosStat->error.amc7891)				ErrorFlag(ERR(abort), (UINT32*)&RX_HBStatus[0].err, err_amc7891, 0, "AMC7891 could not be initialized. Hardware or Power Supply Error.");
		if (_NiosStat->error.power_all_off_timeout)	ErrorFlag(ERR(abort), (UINT32*)&RX_HBStatus[0].err, err_pwr_all_off, 0, "power_all_off_timeout");
		if (_NiosStat->error.power_amp_on_timeout)	ErrorFlag(ERR(abort), (UINT32*)&RX_HBStatus[0].err, err_amp_all_on, 0, "power_amp_on_timeout");
		if (_NiosStat->error.power_all_on_timeout)	ErrorFlag(ERR(abort), (UINT32*)&RX_HBStatus[0].err, err_pwr_all_on, 0, "power_all_on_timeout");
		
		// TODO: re-enable after simu-chiller and cable problem is solved
		//if (_NiosStat->error.chiller_overheated)    ErrorFlag(ERR_ABORT, (UINT32*)&RX_HBStatus[0].err, err_chiller_overheated, 0, "Chiller Overheated Error");
				
		if (_NiosStat->info.cooler_pcb_present)
		{
			if (_NiosStat->error.cooler_temp_hw)		ErrorFlag(WARN,		 (UINT32*)&RX_HBStatus[0].err, err_therm_cooler,      0, "Cooler Thermistor hardware");
    		if (_NiosStat->error.cooler_overheated)	
	    	{
	    		if (ErrorFlag(ERR_ABORT, (UINT32*)&RX_HBStatus[0].err, err_cooler_overheated, 0, "Cooler too hot (%d°C)", _NiosStat->cooler_temp / 1000))
		    	{
		    	//	fpga_overheated();		    		
		    	}
	    	}
			/*
			if (_NiosStat->cooler_pressure > 1200)	
			{				
				if (_CoolerErr<10 && ++_CoolerErr>=10)
				{
	    			Error(WARN,	0, "Cooler Overpressure (%d mBar)", _NiosStat->cooler_pressure);
					RX_HBStatus[0].err |= err_cooler_overpressure;
		    		fpga_overheated();						    										
				}
			}
			else _CoolerErr=0;
			*/
		}
		else
		{
			ErrorFlag(WARN, (UINT32*)&RX_HBStatus[0].info, info_cooler_pcb_present, 0, "No Cooler PCB present");
		}

		cond_error_check(ticks);		
	}
}

//--- nios_error_reset ---------------------------
void nios_error_reset(void)
{
	cond_error_reset();
	tse_error_reset();
	_FPWarning = 0;
	_CoolerErr = 0;
	
	if (_NiosLoaded) _NiosCfg->cmd.error_reset = TRUE;
}

//--- _nios_fastlog_start --------------------------------------------------------------------
#ifdef FASTLOG
static void _nios_fastlog_start(void)
{
	int i = 0;
	// cee
	_FastLog_File  = fopen(PATH_BIN_HEAD "fast_log.csv", "w");
	for (i = 0; i < SIZEOF(RX_HBStatus->head); i++)
		fprintf(_FastLog_File, "Alive %d;Temp %d;Temp Heater %d;Temp Head %d;Heater %% %d;Pres In %d;Pres Out %d;Pump %d;Feedback %d;", i, i, i, i, i, i, i, i, i);
	
	fprintf(_FastLog_File, "Cylinder Pressure;Ink Pump;Feedback;Env Temp\n");
}
#endif

//--- _nios_fastlog ---------------------------------------------------------------
#ifdef FASTLOG	
static void _nios_fastlog(void)
{
	int i = 0;
	int fastlog = 0;
	int t_fastlog = 0;
		
	// 10ms fastlog for finding problems during scan
	int ticks = rx_get_ticks();
	fastlog = ticks > t_fastlog;
	if (fastlog && RX_HBStatus[0].head[0].ctrlMode == ctrl_print)
	{
		t_fastlog = 10*(1 + ticks / 10);
			
		for (i = 0; i < MAX_HEADS_BOARD; i++)
		{
			fprintf(_FastLog_File, "%d;", _NiosStat->cond[i].alive);
			fprintf(_FastLog_File, "%d;", _NiosStat->cond[i].tempIn);
			fprintf(_FastLog_File, "%d;", _NiosStat->cond[i].tempHeater);
			fprintf(_FastLog_File, "%d;", _NiosStat->head_temp[i]);
			fprintf(_FastLog_File, "%s;", value_str_u(_NiosStat->cond[i].heater_percent));
			fprintf(_FastLog_File, "%d;", _NiosStat->cond[i].pressure_in);
			fprintf(_FastLog_File, "%d;", _NiosStat->cond[i].pressure_out);
			fprintf(_FastLog_File, "%d;", _NiosStat->cond[i].pump);
			fprintf(_FastLog_File, "%d;", _NiosStat->cond[i].pump_measured);
		}

		fprintf(_FastLog_File, "%d;", RX_FluidStat[0].cylinderPressure);
		fprintf(_FastLog_File, "%d;", RX_FluidStat[0].inkPump);
		fprintf(_FastLog_File, "%d;", RX_FluidStat[0].inkPumpFeedback);
		fprintf(_FastLog_File, "%d;", RX_FluidStat[0].amcTemp);
			
		fprintf(_FastLog_File, "\n");
		fflush(_FastLog_File);
	}				  
}
#endif // FASTLOG
