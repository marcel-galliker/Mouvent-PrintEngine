// ****************************************************************************
//
//	DIGITAL PRINTING - balance.c
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2013 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include "rx_common.h"
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_term.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "comm.h"
#include "fluid_ctrl.h"
#include "balance.h"

#ifdef linux
	#include <termios.h>
	#include <sys/stat.h> 
	#include <fcntl.h>
#endif

//--- defines --------------------------------------
#define COMM_NAME	"/dev/ttyS1"
#define BUF_SIZE		3

typedef enum
{
	led_off,
	led_on,
	led_flash,
	led_timer,	
} ELedState;

//--- golbals -----------------------------------
static int _Simulation = FALSE;
static int _SimuTime   = 0;

static int _ScaleThreadRunning;
static int _Comm=0;
static RX_SOCKET _Socket = INVALID_SOCKET;

static SScalesCalibration _RX_ScalesCalibration[SCALES_CALIBRATION_CNT];

static SBalanceCfg	_ScalesCfg [BALANCE_CNT];
static SBalanceStat	_ScalesStat [BALANCE_CNT];
static SBalanceStat	_ScalesStatBuf[BUF_SIZE][BALANCE_CNT];
static int			_ScalesStable [BALANCE_CNT];
static ELedState	_LedCfg[BALANCE_CNT];
static int			_Calibrate=-1;
static char			_ScalesSensor [BALANCE_CNT][10];
static SBalanceStat *_pStat;
static int _BufIdx;
static int _CalWeight=0;

//---pprototypes ----------------------------------------
static void *_scl_thread(void *ppar);
static void _comm_set_baud(int comm, int baud);
static void _display_status(void);
static void _process_values(void);
static int  _val_to_g(int scaleNo, int sensorNo);

static void _cal_zero(int no);
static void _cal_10Kg(int scale, int sensor);

static void _simulate(void);

//--- scl_init ----------------------------
int scl_init(void)
{
	SBalanceStat test;
	int i;
	
	printf("RX-Project structure SBalanceStat: size=%d\n", sizeof(test));
	printf("err              : %d\n", (PBYTE)&test.err				 - (PBYTE)&test);
	printf("alive            : %d\n", (PBYTE)&test.alive			 - (PBYTE)&test);
	printf("temp             : %d\n", (PBYTE)&test.temp				 - (PBYTE)&test);
	printf("balance          : %d\n", (PBYTE)&test.balance			 - (PBYTE)&test);
	printf("humidity         : %d\n", (PBYTE)&test.humidity			 - (PBYTE)&test);
	printf("prev_msg_received: %d\n", (PBYTE)&test.prev_msg_received - (PBYTE)&test);
	printf("prev_msg_send    : %d\n", (PBYTE)&test.prev_msg_send	 - (PBYTE)&test);
	printf("next_msg_received: %d\n", (PBYTE)&test.next_msg_received - (PBYTE)&test);
	printf("next_msg_sent    : %d\n", (PBYTE)&test.next_msg_sent	 - (PBYTE)&test);
	
	comm_init(0);
	
	memset(&_ScalesCfg,		0, sizeof(_ScalesCfg));
	memset(&_LedCfg,		0, sizeof(_LedCfg));
	memset(&_ScalesStat,	0, sizeof(_ScalesStat));
	memset(&_ScalesStatBuf,	0, sizeof(_ScalesStatBuf));
	memset(_ScalesStable,   0, sizeof(_ScalesStable));
	for (i=0; i<SIZEOF(_ScalesSensor); i++) strcpy(_ScalesSensor[i], "- ---");
	
	for (i=0; i<SIZEOF(RX_ScaleStatus); i++) RX_ScaleStatus[i].weight = INVALID_VALUE;
	
	_BufIdx   = 0;
	_pStat	  = _ScalesStatBuf[_BufIdx];

	_ScaleThreadRunning = TRUE;
	rx_thread_start(_scl_thread, NULL, 0, "Balance Thread");
	return REPLY_OK;
}

//--- scl_end ------------------------------------
int scl_end(void)
{
	_ScaleThreadRunning = FALSE;
	return REPLY_OK;		
}

//--- scl_cfg_set ----------------------------------
void scl_cfg_set (SScalesCalibration *cfg, int len)
{
	memset(_RX_ScalesCalibration, 0, sizeof(_RX_ScalesCalibration));
	if (len>sizeof(_RX_ScalesCalibration)) len=sizeof(_RX_ScalesCalibration);
	memcpy(_RX_ScalesCalibration, cfg, len);
}

//--- scl_cfg_save --------------------------------------------------
void scl_cfg_save(void)
{
	RX_SOCKET socket=ctrl_socket();
	sok_send_2(&socket, INADDR_ANY, REP_SCALES_SAVE_CFG, sizeof(SScalesCalibration)*BALANCE_CNT, _RX_ScalesCalibration);			
}

//--- scl_cfg_reload --------------------------------------------------
void scl_cfg_reload(void)
{
	RX_SOCKET socket=ctrl_socket();
	sok_send_2(&socket, INADDR_ANY, CMD_SCALES_LOAD_CFG, 0, NULL);			
}

//--- scl_main -------------------------------
int scl_main(int ticks, int menu, int displayStatus)
{
	int i;
	int len, sent;
	UCHAR	buf[COMM_BUF_SIZE];
	
	if (_Simulation) _simulate();
	
	if (_Comm>0 && menu)
	{		
		//--- SEND -------------------------------------------
		if (TRUE)
		{
			for (i=0; i<SIZEOF(_ScalesCfg); i++)
			{
				switch(_LedCfg[i])
				{
				case led_off:		_ScalesCfg[i].cmd.lamp = FALSE;						break;
				case led_on:		_ScalesCfg[i].cmd.lamp = TRUE;						break;
				case led_flash:		_ScalesCfg[i].cmd.lamp = !_ScalesCfg[i].cmd.lamp;	break;
				default:			if (_LedCfg[i]>led_timer) 
									{
										_ScalesCfg[i].cmd.lamp = TRUE;
										if (--_LedCfg[i]==led_timer) _LedCfg[i]=led_off;
									}
				}			
			}
			
			if (comm_encode(_ScalesCfg, BALANCE_CNT*sizeof(SBalanceCfg), buf, sizeof(buf), &len))
			{
				sent = write(_Comm, buf, len);
				if (sent!=len) 
				{
					char buf[128];
					err_system_error(errno, buf, sizeof(buf));
					printf("Error %d: %s\n", errno, buf);
					return REPLY_ERROR;
				}							
			}
		}
		if (displayStatus) _display_status();
	}
	return REPLY_OK;		
}

//--- _scl_thread ------------------------------------
static void *_scl_thread(void *ppar)
{	
	int len, size;
	int comm;
	UCHAR buf[2*sizeof(_ScalesStat)];
	
	//--- open interface ----------------------------
	comm = open(COMM_NAME , O_RDWR | O_NOCTTY);
	if (comm==-1) 
	{
		char str[128];
		err_system_error(errno, str, sizeof(str));
		printf("Error %d: %s\n", errno, str);
		return NULL;
	}

	_comm_set_baud(comm, B9600);

	//--- receiver ---------------------
	_Comm = comm;
	while (_ScaleThreadRunning)
	{
		char data;
		len=read(comm, &data, 1);
		if (comm_received(0, data))
		{
			if (comm_get_data(0, buf, sizeof(buf), &len))
			{
				size=sizeof(_ScalesStat);
				if (len==sizeof(_ScalesStat))
				{
					memcpy(&_ScalesStatBuf[_BufIdx], buf, sizeof(_ScalesStat));
					_pStat = _ScalesStatBuf[_BufIdx];
				//	TrPrintfL(TRUE, "Balance[0]=(%05d %05d %05d)     Balance[1]=(%05d %05d %05d)", _pStat[0].balance[0], _pStat[0].balance[1], _pStat[0].balance[2], _pStat[1].balance[0], _pStat[1].balance[1], _pStat[1].balance[2]);
					_BufIdx = (_BufIdx+1) % BUF_SIZE;
					_process_values();
				}
			}
		}			
	}
	return NULL;		
}

//--- _process_values -------------------------------------------------
static void _process_values(void)
{
	int i, n, m, w, s;
	int val, min, max, sum, cnt;
	int prev, stable, calibrate, caldone;
	int ok;

	//--- convert values ---
	for (i=0; i<BALANCE_CNT; i++)
	{
		_ScalesStat[i].humidity = (_pStat[i].humidity*125/65536)-6;
		_ScalesStat[i].temp	 = (_pStat[i].temp * 175.72 / 65536) - 46.85;
	}
	
	//--- calculate average value -------------
	for (i=0; i<BALANCE_CNT; i++)
	{
		stable		= TRUE;
		calibrate	= FALSE;
		caldone		= TRUE;
		for (n=0; n<SENSOR_CNT; n++)
		{
			sum=0;
			min=100000;
			max=0;
			cnt=0;
			for (m=0; m<BUF_SIZE; m++)
			{
				val = _ScalesStatBuf[m][i].balance[n];
				if (val!=INVALID_VALUE)
				{
					cnt++;
					sum+=val;
					if (val<min) min=val;
					if (val>max) max=val;
				}
			}
//			if (cnt>2) _ScalesStat[i].balance[n] = (sum-min-max)/(cnt-2);
			if (cnt)
			{
				prev =  _ScalesStat[i].balance[n];
				_ScalesStat[i].balance[n]   = sum/cnt;	
				stable &= abs(_ScalesStat[i].balance[n]-prev) < 10;
			}
			else 
			{
				_ScalesStat[i].balance[n] = (UINT16)INVALID_VALUE;
				stable = FALSE;
			}
			if (_RX_ScalesCalibration[i].value_10Kg[n]==0) calibrate=TRUE;
		}
		_ScalesStable[i] = stable;
		if (_Calibrate<0 && _LedCfg[i]<led_timer) _LedCfg[i]=led_off;
		if (_Calibrate==i)
		{
			RX_ScaleStatus[i].weight = INVALID_VALUE;
			if (_LedCfg[i]==led_off) _LedCfg[i] = led_flash;
			if (!calibrate) _Calibrate=-1;
			if (calibrate && stable)
			{
				const int ZERO=500;
				const int NONZERO = 50;
				switch(RX_ScaleStatus[i].state)
				{
				case SCL_CAL_ZERO:  for (n=0, ok=TRUE; n<SENSOR_CNT; n++)
									{
										if (_ScalesStat[i].balance[n]>ZERO) {RX_ScaleStatus[i].sensorState[n] = SCL_SENS_ERROR; ok=FALSE;}
										else							     RX_ScaleStatus[i].sensorState[n] = SCL_SENS_OK;
									}
									if (ok)
									{
										_cal_zero(i);
										RX_ScaleStatus[i].state++;
										for (n=0, ok=TRUE; n<SENSOR_CNT; n++) RX_ScaleStatus[i].sensorState[n] = 0;
										_ScalesSensor[i][0]='*';
										_LedCfg[i] = led_timer+5;
									}
									break;
				case SCL_CAL_SENSOR1:
				case SCL_CAL_SENSOR2:
				case SCL_CAL_SENSOR3: 
									s=RX_ScaleStatus[i].state-SCL_CAL_SENSOR1;
									RX_ScaleStatus[i].sensorState[s] = SCL_SENS_BUSY;
									for (n=0, ok=TRUE; n<SENSOR_CNT; n++)
									{		
										if (s!=n)
										{
											if (_ScalesStat[i].balance[n]-_RX_ScalesCalibration[i].value_0Kg[n]>NONZERO) 
											{
												ok=FALSE;
												if (_RX_ScalesCalibration[i].value_10Kg[n]) RX_ScaleStatus[i].sensorState[n] = SCL_SENS_OK_ERR;
												else                                        RX_ScaleStatus[i].sensorState[n] = SCL_SENS_ERROR;
											}
											else if (_RX_ScalesCalibration[i].value_10Kg[n]) RX_ScaleStatus[i].sensorState[n] = SCL_SENS_OK;
											else											 RX_ScaleStatus[i].sensorState[n] = 0;
										}
									}
									if (ok && _ScalesStat[i].balance[s]-_RX_ScalesCalibration[i].value_0Kg[s]>NONZERO)
									{
										_LedCfg[s] = led_timer+5;
										_ScalesSensor[i][2+s]='*';
										_cal_10Kg(i, s);
										RX_ScaleStatus[i].sensorState[s] = SCL_SENS_OK;
										RX_ScaleStatus[i].state++;
									}
									break;
						
				case SCL_CAL_OK:	
									break;
					
				default:			break;
					
				}
			}			
		}
		else if (calibrate) 
		{
			RX_ScaleStatus[i].weight = INVALID_VALUE;
			strcpy(_ScalesSensor[i], "ERROR");
		}
		else 
		{
			INT32 weight = scl_weight(i);
			if (weight>0) RX_ScaleStatus[i].weight = weight;
			else          RX_ScaleStatus[i].weight = 0;
			strcpy(_ScalesSensor[i], "OK");
		}
	}
}

//--- _comm_set_baud --------------------
static void _comm_set_baud(int comm, int baud)
{
#ifdef linux
	struct termios options;
		
	tcgetattr(comm, &options);
	cfsetispeed(&options, baud);
	cfsetospeed(&options, baud);
	
	//mode: 8n1
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag |= CREAD;
	
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	
	tcsetattr(comm, TCSANOW, &options);
#endif
}

//--- scl_weight -----------------------------------
int scl_weight(int no)
{
	int i, val, weight;

	if (no<0 || no>=SIZEOF(_ScalesStat)) return INVALID_VALUE;
	
	for (i=0, weight=0; i<SENSOR_CNT; i++)
	{
		val=_val_to_g(no, i);
		if (val==INVALID_VALUE) return INVALID_VALUE;
		weight += val;
	}
	return weight;
}

//--- _val_to_g ----------------------------------
static int _val_to_g(int scaleNo, int sensorNo)
{
	int diff;

	if (_RX_ScalesCalibration[scaleNo].value_10Kg[sensorNo]==0) return INVALID_VALUE;
	diff = _RX_ScalesCalibration[scaleNo].value_10Kg[sensorNo] - _RX_ScalesCalibration[scaleNo].value_0Kg[sensorNo];
	if (_ScalesStat[scaleNo].balance[sensorNo]==INVALID_VALUE) return INVALID_VALUE;
	if (diff<10) return 0;
	return 10000 * (_ScalesStat[scaleNo].balance[sensorNo] - _RX_ScalesCalibration[scaleNo].value_0Kg[sensorNo]) / diff;				
}

//--- scl_humidity ----------------------------------------
int scl_humidity(int no)
{
	return _ScalesStat[no].humidity;		
}

//--- _display_status --------------------------
static void _display_status(void)
{
	int i;
	char str[8]; 
	
	term_printf("\n");
	term_printf("--- Balance  ------"); for (i=0; i<BALANCE_CNT; i++) term_printf("  ---%d---", i); term_printf("\n");
	term_printf("PCB Revision:      "); for (i=0; i<BALANCE_CNT; i++){sprintf(str, "%c", _pStat->board_revision);term_printf("   %1s     ", str);} term_printf("\n");
	term_printf("alive:             ");	for (i=0; i<BALANCE_CNT; i++) term_printf("   %05d ", _pStat[i].alive);					term_printf("\n");
	term_printf("Temperature:       ");	for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_pStat[i].temp));		term_printf("\n");
	term_printf("Humidity [%]:      "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_pStat[i].humidity));	term_printf("\n");
	term_printf("--- Calibration 0Kg ----------------------------------------------------------------------\n");
	term_printf("calib State:       "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8d ", RX_ScaleStatus[i].state);		term_printf("\n");
	term_printf("sensor[0]:         "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_RX_ScalesCalibration[i].value_0Kg[0]));		term_printf("\n");
	term_printf("sensor[1]:         "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_RX_ScalesCalibration[i].value_0Kg[1]));		term_printf("\n");
	term_printf("sensor[2]:         "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_RX_ScalesCalibration[i].value_0Kg[2]));		term_printf("\n");
	term_printf("--- Calibration 10Kg (weight=%s Kg) ---------------------------------------------------\n", value_str3(_CalWeight));
	term_printf("sensor[0]:         "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_RX_ScalesCalibration[i].value_10Kg[0]));	term_printf("\n");
	term_printf("sensor[1]:         "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_RX_ScalesCalibration[i].value_10Kg[1]));	term_printf("\n");
	term_printf("sensor[2]:         "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_RX_ScalesCalibration[i].value_10Kg[2]));	term_printf("\n");
//	term_printf("--- measured values ----------------------------------------------------------------------\n");
//	term_printf("sensor[0]:         "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_pStat[i].balance[0]));		term_printf("\n");
//	term_printf("sensor[1]:         "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_pStat[i].balance[1]));		term_printf("\n");
//	term_printf("sensor[2]:         "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_pStat[i].balance[2]));		term_printf("\n");
	term_printf("--- Value --------------------------------------------------------------------------------\n");
	term_printf("sensor[0]:         "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_ScalesStat[i].balance[0]));		term_printf("\n");
	term_printf("sensor[1]:         "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_ScalesStat[i].balance[1]));		term_printf("\n");
	term_printf("sensor[2]:         "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_ScalesStat[i].balance[2]));		term_printf("\n");
	term_printf("stable:            "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_ScalesStable[i]));					term_printf("\n");
	term_printf("sensor:            "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", _ScalesSensor[i]);							term_printf("\n");
	term_printf("--- converted to [g] ---------------------------------------------------------------------\n");
	term_printf("sensor[0]:         "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_val_to_g(i,0)));		term_printf("\n");
	term_printf("sensor[1]:         "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_val_to_g(i,1)));		term_printf("\n");
	term_printf("sensor[2]:         "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str(_val_to_g(i,2)));		term_printf("\n");
	term_printf("--- Weight -------------------------------------------------------------------------------\n");
	term_printf("weight [kg]: (avg) "); for (i=0; i<BALANCE_CNT; i++) term_printf("%8s ", value_str3(scl_weight(i))); term_printf("\n");
	
	/*	
	term_printf("\n");
	term_printf("next: msg sent     ");	for (i=0; i<BALANCE_CNT; i++) term_printf("     %03d ", _pStat->next_msg_sent);		term_printf("\n");
	term_printf("next: msg received ");	for (i=0; i<BALANCE_CNT; i++) term_printf("     %03d ", _pStat->next_msg_received);	term_printf("\n");
	term_printf("prev: msg sent     ");	for (i=0; i<BALANCE_CNT; i++) term_printf("     %03d ", _pStat->prev_msg_send);		term_printf("\n");				
	term_printf("prev: msg received ");	for (i=0; i<BALANCE_CNT; i++) term_printf("     %03d ", _pStat->prev_msg_received);	term_printf("\n");
	term_printf("prev: pump_fault   "); for (i=0; i<BALANCE_CNT; i++) term_printf("       %01d ", _pStat->err.pump);			term_printf("\n");	
	term_printf("Blink Test:        "); for (i=0; i<BALANCE_CNT; i++) term_printf("       %01d ", _ScalesCfg[i].cmd.lamp);					term_printf("\n");
	*/
}

//--- scl_lamp -----------------------------------------------
void scl_lamp(int no)
{
	_ScalesCfg[no].cmd.lamp = !_ScalesCfg[no].cmd.lamp;
}

//--- scl_calibrate --------------------------------
void scl_calibrate(SScalesCalibrateCmd *pcmd)
{
	int i, n;
	memset(RX_ScaleStatus, 0, sizeof(RX_ScaleStatus));
	_Calibrate=-1;

	_CalWeight = pcmd->weight;

	if (pcmd->no>=0 && pcmd->no<BALANCE_CNT) 
	{
		_Calibrate = pcmd->no;
		RX_ScaleStatus[_Calibrate].state = SCL_CAL_ZERO;
		_SimuTime = 0;
		for (i=0; i<SIZEOF(RX_ScaleStatus); i++)
		{
			for (n=0; n<SENSOR_CNT; n++) RX_ScaleStatus[i].sensorState[n] = 0;				
		}
		memset(&_RX_ScalesCalibration[_Calibrate], 0, sizeof(_RX_ScalesCalibration[_Calibrate]));
		strcpy(_ScalesSensor[_Calibrate], "- ---");
	}
}

//--- _cal_zero -------------------------------------------------
void _cal_zero(int no)
{
	int n;
	if (no>=0 && no<SIZEOF(_RX_ScalesCalibration))
	{
		for (n=0; n<SENSOR_CNT; n++)
		{
			_RX_ScalesCalibration[no].value_0Kg[n]  = _ScalesStat[no].balance[n];
			_RX_ScalesCalibration[no].value_10Kg[n] = 0; 				
		}
	}
}

//--- _cal_10Kg -------------------------------------------------
void _cal_10Kg(int i, int n)
{
	int diff;
	if (i>=0 && i<SIZEOF(_RX_ScalesCalibration) && _CalWeight>=2000)
	{
		diff = _ScalesStat[i].balance[n] - _RX_ScalesCalibration[i].value_0Kg[n];
		_RX_ScalesCalibration[i].value_10Kg[n] = _RX_ScalesCalibration[i].value_0Kg[n] + (UINT16)((diff * 10000)/_CalWeight); 
	}		
}

//--- _simulate ------------------------------------------------
static void _simulate(void)
{
	static int time0;
	SScaleStat *pstat =	&RX_ScaleStatus[_Calibrate];
	
	if (_Calibrate<0) _SimuTime=0;
	else
	{
		int t = rx_get_ticks();
		if (t-time0>10)
		{
			_SimuTime += 10;
			time0  = t;
			switch(_SimuTime)
			{
			case   100:	pstat->sensorState[1] = SCL_SENS_ERROR; break;
		
			case  1000:	pstat->sensorState[1] = 0; 
						pstat->state		  = SCL_CAL_SENSOR1;
						pstat->sensorState[0] = SCL_SENS_BUSY;
						break;
		
			case  4000: pstat->sensorState[0] = SCL_SENS_OK;
						pstat->state		  = SCL_CAL_SENSOR2;
						pstat->sensorState[1] = SCL_SENS_BUSY;
						break;
		
			case  8000: pstat->sensorState[1] = SCL_SENS_OK;
						pstat->state		  = SCL_CAL_SENSOR3;
						pstat->sensorState[2] = SCL_SENS_BUSY;
						break;
		
			case 12000:	pstat->sensorState[2] = SCL_SENS_OK;
						pstat->state		  = SCL_CAL_OK;
						break;
			}					
		}
	}
}

