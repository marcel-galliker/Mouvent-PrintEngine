// ****************************************************************************
//
//	datalogic.c	
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#ifdef linux
	#include <termios.h>
#endif
#include "rx_common.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_threads.h"
#include "errno.h"
#include "setup.h"
#include "datalogic.h"

//--- defines ------------------------------------
#define DEVICE_NAME_BASE "/dev/ttyACM"
#define IDENTIFY_STR	"Mouvent AG: Barcode Scanner Identification"

//--- global  variables --------------------------

typedef struct 
{
	int		handle;
	int		isNo;
	char	scannerSN[16];
	char	code[256];
} SScanner;	

static SScanner _Scanner[INK_SUPPLY_CNT];
static char		_Configured[INK_SUPPLY_CNT][16];
static int		_Identify;

//--- prorotypes ---------------------------------
static void *_dl_thread(void *lpParameter);

static void _dl_get_serialNo(char *deviveName, char *serialNo);
static void _dl_set_baud	(int handle, int baud);
static void _dl_write		(int handle, char *str);
static void _dl_write_str	(int handle, char *str, char *par, int parlen);
static void _dl_read		(int handle);
static void _dl_configure	(int handle);
static void _dl_reset		(int handle);

static int _configured_add(char *serialNo);


//--- dl_init ------------------------------------------
void dl_init(void)
{
	int i;
	_Identify = -1;
	memset(_Scanner, 0, sizeof(_Scanner));
	memset(_Configured, 0, sizeof(_Configured));
	for (i=0; i<SIZEOF(_Scanner); i++)
		_Scanner[i].isNo = -1;
	rx_thread_start(_dl_thread, NULL, 0, "_datalogic_thread");		
}

//--- _dl_get_serialNo ----------------------------------------
static void _dl_get_serialNo(char *deviveName, char *serialNo)
{
#ifdef linux
	FILE* fp;
	char str[100];
	char *ch, *dst;
	dst=serialNo;
	sprintf(str, "udevadm info -a -p  $(udevadm info -q path -n \"%s\") | grep serial | grep S/N", deviveName);
	fp = popen(str, "r");
	if (fp)
	{
		fgets(str, sizeof(str), fp);
		for (ch=str; *ch; )
		{
			if (*ch++=='"')
			{
				while(*ch && *ch!='"') 
					*dst++ = *ch++;
				break;
			}
		}
		pclose(fp);
	}
	*dst=0;
#endif
}

//--- _configured_add ------------------------------------------------
static int _configured_add(char *serialNo)
{
	int i;
	for (i=0; i<SIZEOF(_Configured); i++)
	{
		if (!strcmp(_Configured[i], serialNo)) return FALSE;
		if (!_Configured[i][0])
		{
			strcpy(_Configured[i], serialNo);
			return TRUE;		
		}
	}
	return FALSE;
}

//--- _dl_thread ---------------------------------------------------
static void *_dl_thread(void *lpParameter)
{
#ifdef linux
	while (TRUE)
	{
		rx_sleep(1000);
	
		//--- search interface name --------------
		{
			int len, no;
			int i, n, found;
			char buf[256];
			char serialNo[32];
			SEARCH_Handle search;
			
			search = rx_search_open("/dev", "ttyACM*");
			found=0;
			while (rx_search_next(search, buf, sizeof(buf), NULL, NULL, NULL))
			{
				if (sscanf(buf, "ttyACM%d", &no) && no<SIZEOF(_Scanner) )
				{
					found |= (1<<no);
					if (_Scanner[no].handle<=0)
					{
						sprintf(buf, "/dev/ttyACM%d", no);
						_dl_get_serialNo(buf, _Scanner[no].scannerSN);
						for (i=0; i<SIZEOF(RX_Config.inkSupply); i++) 
						{
							_configured_add(RX_Config.inkSupply[i].scannerSN);
							if (!strcmp(_Scanner[no].scannerSN, RX_Config.inkSupply[i].scannerSN))
								_Scanner[no].isNo=i;													
						}
						_Scanner[no].handle = open(buf, O_RDWR);

						if(_Scanner[no].handle == -1) 
						{
							err_system_error(errno, buf, sizeof(buf));
							printf("Error %d: %s\n", errno, buf);
						}
						else
						{	
							if(_configured_add(_Scanner[no].scannerSN))
							{								
							//	_dl_configure(_Scanner[no].handle);
							}

							//--- set non blocking ---------------------------------------
							int flags = fcntl(_Scanner[no].handle, F_GETFL, 0);
							fcntl(_Scanner[no].handle, F_SETFL, flags | O_NONBLOCK);
							
							if (_Identify>=0) write(_Scanner[no].handle, "ON", 2);
						}
					}
				}
			}
			rx_search_close(search);
			
			for (no=0; no<SIZEOF(_Scanner); no++)
			{
				if (_Scanner[no].handle>0)
				{
					if (found&(1<<no))
					{
						memset(buf, 0, sizeof(buf));
						len = read(_Scanner[no].handle, buf, sizeof(buf));							
						if(len>0)
						{
							buf[len-1]=0;
							if (_Identify>=0 && !strcmp(buf, IDENTIFY_STR))
							{
								ErrorEx(dev_fluid, _Identify, LOG, 0, "Identified Barcode Scanner >>%s<<", _Scanner[no].scannerSN);
								strcpy(RX_Config.inkSupply[_Identify].scannerSN, _Scanner[no].scannerSN);
								{
									SRxConfig cfg;
									setup_config(PATH_USER FILENAME_CFG, &cfg, READ);
									strcpy(cfg.inkSupply[_Identify].scannerSN, _Scanner[no].scannerSN);
									setup_config(PATH_USER FILENAME_CFG, &RX_Config, WRITE);
								}								
								_Identify = -1;
								_dl_configure(_Scanner[no].handle);
								for (i=0; i<SIZEOF(_Scanner); i++)
								{
									if (_Scanner[i].handle>0) 
									{
										write(_Scanner[i].handle, "OFF", 3);										
										close(_Scanner[i].handle);
										_Scanner[i].handle	= 0;
									}
								}		
							}
							else
							{
									ErrorEx(dev_fluid, _Scanner[no].isNo, LOG, 0, "TTY%d: Got Barcode >>%s<<", no, buf);
									strcpy(_Scanner[no].code, buf);																	
								write(_Scanner[no].handle, "OFF", 3);
								close(_Scanner[no].handle);
								_Scanner[no].handle	= 0;	
							}
						}							
					}
					else
					{
						printf("Lost [%d]\n", no);
						_Scanner[no].handle	= 0;	
					}
				}
			}
		}
	}
#endif
	return NULL;
}

//--- dl_trigger --------------------------------------
void dl_trigger(int isNo)
{	
	int i;
	printf("dl_trigger(isNo=%d)\n", isNo);
	for (i=0; i<SIZEOF(_Scanner); i++)
	{
		if (_Scanner[i].handle>0 && (isNo<0 || _Scanner[i].isNo==isNo)) 
		{
			printf("Scanner[%d]: Write >>ON<<\n", i);
			write(_Scanner[i].handle, "ON", 2);			
	}
	}
}	

//--- dl_identify ---------------------------------------------
void dl_identify(int isNo)
{
	_Identify = isNo;
	if(isNo < 0)
	{
		int i;
		for (i=0; i<SIZEOF(_Scanner); i++)
		{
			if (_Scanner[i].handle>0) 
			{
				write(_Scanner[i].handle, "OFF", 3);										
				close(_Scanner[i].handle);
				_Scanner[i].handle	= 0;
			}
		}		
	}
	else	dl_trigger(-1);
}

//--- dl_reset --------------------------------------
void dl_reset(int isNo)
{	
	int i;
	for (i=0; i<SIZEOF(_Scanner); i++)
	{
		if (_Scanner[i].handle>0 && (isNo<0 || _Scanner[i].isNo==isNo))
		{
			_dl_reset(_Scanner[i].handle);						
			write(_Scanner[i].handle, "OFF", 3);										
			close(_Scanner[i].handle);
			_Scanner[i].handle	= 0;
		}
	}
}	

//--- dl_get_barcode ---------------------------------------------
void dl_get_barcode(int isNo, char *scannerSN, char *barcode)
{
	int i;
	*barcode=0;
	*scannerSN=0;
	for (i=0; i<SIZEOF(_Scanner); i++)
	{
		if (_Scanner[i].handle>0 && (isNo<0 || _Scanner[i].isNo==isNo)) 
		{
			TrPrintfL(TRUE, "Copy Barcode isNo=%d", isNo);
			strncpy(scannerSN, _Scanner[i].scannerSN, sizeof(_Scanner[i].scannerSN));
			strncpy(barcode,   _Scanner[i].code, sizeof(_Scanner[i].code));
			return;
		}			
	}
}

//--- _dl_configure -------------------------------------------------
static void _dl_configure(int handle)
{
#ifdef linux
	char str[10];
	int flags = fcntl(handle, F_GETFL, 0);
	fcntl(handle, F_SETFL, flags & ~O_NONBLOCK);

	rx_sleep(100);
	
	//--- Sevice Mode (RefManual400.pdf p218) ----------------------------------
	_dl_set_baud(handle, B9600);
	_dl_write(handle, "$S\n");							// enter service mode
	_dl_set_baud(handle, B115200);
	_dl_write(handle, "$S\n");							// enter service mode

	//--- Reading Parameters (RefManual400.pdf p224) ---------------------------
	_dl_write(handle, "$CSNRM01\n");					// Operating Mode: Serial On Line
	_dl_write(handle, "$CSPIL02\n");					// Illumination: Endabled
	_dl_write(handle, "$CSPTO00\n");					// Phase Off Event: Trigger Stop		
	_dl_write_str(handle, "$CSTON", "ON",  20);			// 
	_dl_write_str(handle, "$CSTOF", "OFF", 20);
	_dl_write_str(handle, "$CLFPR", "", 20);
		
	//--- LED and Beeper configuraion (RefManual400.pdf p228) -------------------		
	_dl_write(handle, "$CBPPU00\n");					// Power Up Beep:  Disable
	_dl_write(handle, "$CBPVO00\n");					// Good Read Beep: Disable
//	_dl_write(handle, "$CLSSP00\n");					// Green Spot Duration: Disable
		
	_dl_write(handle, "$Ar\n");							// Save and exit service mode / Write to FLASH
//	_dl_write(handle, "$r01\n");						// Save and exit service mode / in RAM				
	
	fcntl(handle, F_SETFL, flags);
	rx_sleep(500);
#endif
}

//--- _dl_reset ------------------------------------
static void _dl_reset(int handle)
{
#ifdef linux
	rx_sleep(100);
	
	//--- Sevice Mode (RefManual400.pdf p218) ----------------------------------
	_dl_set_baud(handle, B115200);
	_dl_write(handle, "$S\n");							// enter service mode				
	_dl_write(handle, "R\n");							// enter service mode				
#endif
}

//--- _dl_write -----------------------------------------------
static void _dl_write(int handle, char *str)
{
	int len;
	char buf[256];

	strcpy(buf, str);
	buf[strlen(buf)-1]=0;
	printf("Request: >>%s<<\n", buf);
	len = (int)strlen(str);
	len = write(handle, str, len);
	if (len<0) 
	{
		err_system_error(errno, buf, sizeof(buf));
		printf("Error err=%d: >>%s<<\n", errno, buf);
	}
	rx_sleep(20);
	_dl_read(handle);
}

//--- _dl_write_str --------------------------------------------
static void _dl_write_str(int handle, char *str, char *par, int parlen)
{
	char buf[256];
	int len, i;
	
	len =(int)strlen(str);
	strcpy(buf, str);
	for(i=0; i<parlen; i++)
	{
		if (*par) len += sprintf(&buf[len], "%02X", (UCHAR)*par++);
		else      len += sprintf(&buf[len], "00");
	}
	sprintf(&buf[len], "\n");
	_dl_write(handle, buf);
}

//--- _dl_read ----------------------------------
static void  _dl_read(int handle)
{
	int	len=0;
	char buf[256];
	printf("Response: ");
	while (len == 0)
	{
		memset(buf, 0, sizeof(buf));
		len=read(handle, buf, sizeof(buf));
		if (len>0)
		{
			buf[len-1]=0;
			printf(">>%s<<", buf);
		}
	}
	printf("\n");
}

//--- _dl_set_baud --------------------
static void _dl_set_baud(int handle, int baud)
{
#ifdef linux
	struct termios options;
		
	tcgetattr(handle, &options);
	cfsetispeed(&options, baud);
	cfsetospeed(&options, baud);
	
	//mode: 8n1
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag |= CREAD;
	
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	
	tcsetattr(handle, TCSANOW, &options);
#endif
}