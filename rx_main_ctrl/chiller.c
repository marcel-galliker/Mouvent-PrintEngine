// ****************************************************************************
//
//	chiller.c		
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#ifdef linux
	#include <termios.h>
#endif
#include "rx_common.h"
#include "rx_error.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "args.h"
#include "chiller.h"

//--- telegrams ------------------------------------------------
#define DEVICE_NAME	"ttyUSB0"

static const char *_convert="0123456789ABCDEF";

#define READ_REGISTERS	0x03
#define WRITE_REGISTER	0x06

#define CHILLER_TEMP_SET		280	// temperature in 0.1°C
#define CHILLER_TEMP_SET_UV		400	// temperature in 0.1°C

#define TIMEOUT 2

//--- prototypes -----------------------------------------------
static void _set_baud_7e1(int baud);
static INT32 _read_register(int no);
static int	 _read_registers(int no, int cnt, INT16 *pregisters);
static int   _write_register(int no, INT16 val);
static int  _read_data(int function, INT16 *data, int cnt);
static void _send_data(int function, INT16 *data, int len);
static void _set_temp(int temp);

static void *_chiller_thread(void *lpParameter);

//--- global  variables --------------------------
static SChillerStat _ChillerStatus;
static INT64		_ErrorFlags;	
static INT64		_Alarm;
static int			_Timeout;
static int			_TempMax=30;

static int			_ChillerThreadRunning;
static int			_Handle=0;

static char*_ChillerAlarmStr[] = 
	{
		//--- Alarm flag 1 ---
		"Low level in tank",										// 0x00	
		"High circulating fluid discharge temp.",					// 0x01	
		"Circulating fluid discharge temp. rise",					// 0x02	
		"Circulating fluid discharge temp.",						// 0x03	
		"High circulating fluid return temp.",						// 0x04	
		"High circulating fluid discharge pressure",				// 0x05	
		"Abnormal pump operation",									// 0x06	
		"Circulating fluid discharge pressure rise",				// 0x07	
		"Circulating fluid discharge pressure drop",				// 0x08	
		"High compressor intake temp.",								// 0x09	
		"Low compressor intake temp.",								// 0x0a	
		"Low super heat temperature",								// 0x0b	
		"High compressor discharge pressure",						// 0x0c	
		"",															// 0x0d	
		"Refrigerant circuit pressure (high pressure side) drop",	// 0x0e	
		"Refrigerant circuit pressure (low pressure side) rise",	// 0x0f	
		
		//--- Alarm flag 2 ---
		"Refrigerant circuit pressure (low pressure side) drop",	// 0x10	
		"Compressor overload",										// 0x11	
		"Communication error",										// 0x12	
		"Memory error",												// 0x13	
		"DC line fuse cut",											// 0x14	
		"Circulating fluid discharge temp. sensor failure",			// 0x15	
		"Circulating fluid return temp. sensor failure",			// 0x16	
		"Compressor intake temp. sensor failure",					// 0x17	
		"Circulating fluid discharge pressure sensor failure",		// 0x18	
		"Compressor discharge pressure sensor failure",				// 0x19	
		"Compressor intake pressure sensor failure",				// 0x1a	
		"Maintenance of pump",										// 0x1b	
		"Maintenance of fan motor",									// 0x1c	
		"Maintenance of compressor",								// 0x1d	
		"Contact input 1 signal detection alarm",					// 0x1e	
		"Contact input 2 signal detection alarm",					// 0x1f	
		
		//--- Alarm flag 3 ---
		"Water leakage",											// 0x20	
		"DI level rise",											// 0x21	
		"DI level drop",											// 0x22	
		"DI sensor error",											// 0x23		
		"",															// 0x24
		"",															// 0x25	
		"",															// 0x26	
		"",															// 0x27	
		"",															// 0x28	
		"",															// 0x29	
		"",															// 0x2a	
		"",															// 0x2b	
		"",															// 0x2c	
		"",															// 0x2d	
		"",															// 0x2e	
		
		//--- Alarm flag internal ---
		"Chiller not in Serial mode",								// 0x2f	
		"USB Converter not found",									// 0x30
		"Communication Timeout",									// 0x31
	};

//--- chiller_init -------------------------------------
int	 chiller_init(void)
{
	chiller_error_reset();
	_ChillerThreadRunning = TRUE;
#ifdef linux
	rx_thread_start(_chiller_thread, NULL, 0, "ChillerThread");		
#endif
	return REPLY_OK;
}

//--- chiller_set_temp -----------------------------------------
void chiller_set_temp(int tempMax)
{
	_TempMax = tempMax;
	if(arg_simuChiller) 
	{
		_ChillerStatus.enabled = FALSE;
		Error(WARN,0,"Chiller in Simulation");
		return;			
	}
	else
	{
		_ChillerStatus.enabled = (RX_Config.printer.type>=printer_LB701);
		
		#ifndef linux
		if(_ChillerStatus.enabled) Error(ERR_CONT, 0, "Chiller not implemented in Windows version");
		#endif
	}
}

//--- chiller_end --------------------------------------
int  chiller_end(void)
{
	return REPLY_OK;		
}

//--- chiller_tick -------------------------------------
void chiller_tick(void)
{
	int i;
	UINT64 err;
	UINT64 flag;
	
	if (_ChillerStatus.enabled)
	{	
		if(_Timeout > 0)
		{
			if (--_Timeout == 0) _Alarm |= ((UINT64)1 << 0x31);
		}
		else if(_Handle <= 0) _Alarm |= ((UINT64)1 << 0x30);
		else if (!(_ChillerStatus.status & 0x0020)) _Alarm |= ((UINT64)1 << 0x2f);
			
		err = _Alarm & ~_ErrorFlags;
		if (err)
		{
			for (i=0, flag=1; i<SIZEOF(_ChillerAlarmStr); i++, flag<<=1)
			{
				if (err & flag)
				{
					Error(ERR_CONT, 0, "Chiller: %s", _ChillerAlarmStr[i]);
					_ErrorFlags |= flag;
				}
			}	
		}			
	}
}

//--- chiller_is_running -----------------------------------
int chiller_is_running(void)
{
	if (!_ChillerStatus.enabled) return TRUE;
	return _ChillerStatus.running;
}

//--- chiller_error_reset ------------------------------------
void chiller_error_reset(void)
{
	_ErrorFlags = 0;
	_Timeout    = TIMEOUT;
}

//--- fluid_reply_stat ------------------------------------
void chiller_reply_stat(RX_SOCKET socket)	// to GUI
{
	sok_send_2(&socket, REP_CHILLER_STAT, sizeof(_ChillerStatus), &_ChillerStatus);
}

//--- _read_register -------------------------------------------
static INT32 _read_register(int no)
{
	INT16 data[2];
	INT16 reply;
	data[0]=no;
	data[1]=1;
	_send_data(READ_REGISTERS, data, SIZEOF(data));
	if (_read_data(READ_REGISTERS, &reply, 1) == REPLY_OK) return (INT32)reply;
	return INVALID_VALUE;
}

//--- _read_registers ----------------------------
static int _read_registers(int no, int cnt, INT16 *pregisters)
{
	INT16 data[2];
	data[0]=no;
	data[1]=cnt;
	_send_data(READ_REGISTERS, data, SIZEOF(data));
	return _read_data(READ_REGISTERS, pregisters, cnt);
}

//--- _write_register ---------------------------------
static int _write_register(int no, INT16 val)
{
	INT16 data[2];
	INT16 reply;
	data[0]=no;
	data[1]=val;
	_send_data(WRITE_REGISTER, data, SIZEOF(data));
	if (_read_data(WRITE_REGISTER, &reply, 1) == REPLY_OK) return reply;
	else return INVALID_VALUE;
}

//---_set_temp -----------------------------------
static void _set_temp(int temp)
{
	_ChillerStatus.tempSet = _write_register(0x0b, temp);	// temperature
}

//--- _chiller_thread -------------------------------
#ifdef linux
static void *_chiller_thread(void *lpParameter)
{
	char buf[256];
	int i, ret;
	int err=FALSE;
	INT16 registers[8];
	
	while (_ChillerThreadRunning)
	{
		if (_ChillerStatus.enabled)
		{
			ret = REPLY_ERROR;
			if (_Handle<=0)
			{
				_Handle = open("/dev/" DEVICE_NAME, O_RDWR);
				if (_Handle>0)
				{
					_set_baud_7e1(B19200);							
					_Timeout = TIMEOUT;
				}
				else if (!err) 
				{
					err=TRUE;
					err_system_error(errno, buf, sizeof(buf));
					TrPrintfL(TRUE, "Chiller: Error %d: %s\n", errno, buf);
				}
			}
			if (_Handle>=0)
			{
				if (_TempMax>36)	_set_temp(CHILLER_TEMP_SET_UV);
				else				_set_temp(CHILLER_TEMP_SET);
				_write_register(0x0c, 1);											// Operation START				
				ret = _read_registers(0x00, 8, registers);		
			}
			if (ret == REPLY_OK)
			{
				_ChillerStatus.temp			= registers[0x00];
				_ChillerStatus.pressure		= registers[0x02];
				_ChillerStatus.resistivity	= registers[0x03];
				_ChillerStatus.running		= registers[0x04] & 0x0001;
				_ChillerStatus.status	    = registers[0x04];
				_Alarm						= (registers[0x05] << 16) | (registers[0x06] << 8) | (registers[0x07]);

				_Timeout = 0;
				/*
				printf("Temp: %02d.%d(%02d.%d)   pres: %0d    Status: 0x%04X   Restivity: %d\n", 
					_ChillerStatus.temp/10,
					_ChillerStatus.temp%10, 
					_ChillerStatus.tempSet/10,
					_ChillerStatus.tempSet%10, 
					_ChillerStatus.pressure, 
					_ChillerStatus.status, 
					_ChillerStatus.resistivity);
				*/	
			}
			else
			{
				_ChillerStatus.temp			= INVALID_VALUE;
				_ChillerStatus.pressure		= INVALID_VALUE;
				_ChillerStatus.resistivity	= registers[0x03];
				_ChillerStatus.running		= FALSE;
			}
		}

		rx_sleep(1000);
	}
}
#endif

//--- _send_data ------------------------
static void _send_data(int function, INT16 *data, int len)
{
	char buf[256];
	char test[256];
	char val;
	int i;
	UCHAR lrc;

	sprintf(test, ":010300000001FB\r\n");
	memset(buf, 0, sizeof(buf));
	buf[0]=0x3a;		// start
	buf[1]='0';			// Slave Addr x.
	buf[2]='1';			// Slave Addr .x
	buf[3]=_convert[(function>>4)&0x0f]; // Function   x.
	buf[4]=_convert[ function    &0x0f]; // Function   .x
	for (i=0; i<len; i++)
	{
		buf[5+4*i]=_convert[(data[i]>>12) &0x0f]; // Function   x.
		buf[6+4*i]=_convert[(data[i]>> 8) &0x0f]; // Function   .x
		buf[7+4*i]=_convert[(data[i]>> 4) &0x0f]; // Function   x.
		buf[8+4*i]=_convert[(data[i]>> 0) &0x0f]; // Function   .x
	}
	for (i=1, lrc=0; i<9+4*(len-1); i+=2) 
	{
		val = ((strchr(_convert, buf[i])-_convert)<<4) | (strchr(_convert, buf[i+1])-_convert);
		lrc+=val;
	}
	lrc=~lrc+1;
	len = 9+4*(len-1);
	buf[len++]=_convert[(lrc>>4)&0x0f];		// LRC
	buf[len++]=_convert[ lrc    &0x0f];		// LRC
	buf[len++]='\r';	// CR
	buf[len++]='\n';	// LF
	write(_Handle, buf, len);
}

//--- _read_data ----------------------------------
static int  _read_data(int function, INT16 *data, int cnt)
{
	int	idx;
	int l, len;
	char buf[256];
	UCHAR msg[128];
	
	struct timeval timeout;
	fd_set readSet;

	int tio=1000;
	timeout.tv_sec  = tio/1000;
	timeout.tv_usec = 1000*(tio%1000);
	
	if (function == READ_REGISTERS) len = 11 + 4*cnt;
	else if (function==WRITE_REGISTER) len=17;
	for(idx=0; idx<len; idx+=l)
	{
		FD_ZERO(&readSet);
		FD_SET(_Handle, &readSet);
		if (select(_Handle + 1, &readSet, NULL, NULL, &timeout)==0) 
			return REPLY_ERROR; 
		l=read(_Handle, &buf[idx], len-idx);
		if (l<0) return REPLY_ERROR;
	}
	buf[idx-2]=0;
	for (idx=1; idx<len; idx+=2)
	{
		msg[idx/2] = ((strchr(_convert, buf[idx])-_convert)<<4) | (strchr(_convert, buf[idx+1])-_convert);
	}
	// msg[0]: Address
	// msg[1]: Function
	if (msg[1] == READ_REGISTERS)
	{
		// msg[2]: Bytes
		// msg[3] ...: data
		if (msg[2] != 2*cnt) 
			return REPLY_ERROR;
		for (idx = 0; idx < cnt; idx++)
			data[idx] = (msg[3 + 2*idx] << 8) | msg[4 + 2*idx];
	}
	else if (msg[1]==WRITE_REGISTER)
	{		
		data[0] = (msg[4] << 8) | msg[5];
	}
	return REPLY_OK;
}

//--- _set_baud_7e1 --------------------
static void _set_baud_7e1(int baud)
{
#ifdef linux
	struct termios options;
		
	tcgetattr(_Handle, &options);
	cfsetispeed(&options, baud);
	cfsetospeed(&options, baud);
	
	//mode: 7e1
	options.c_cflag |= PARENB;
	options.c_cflag &= ~PARODD;	
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS7;
	options.c_cflag |= CREAD;
	options.c_cflag |= baud;
	
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	
	tcsetattr(_Handle, TCSANOW, &options);
#endif
}
