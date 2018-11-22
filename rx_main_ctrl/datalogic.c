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
#include "rx_threads.h"
#include "errno.h"
#include "datalogic.h"

//--- defines ------------------------------------

//--- global  variables --------------------------
static int _Handle=0;

//--- prorotypes ---------------------------------
static void _dl_write(char *str);
static void _dl_write_str(char *str, char *par, int parlen);
static void _dl_read(void);
static void _dl_set_baud(int baud);

//--- dl_test -------------------------------------
void dl_test(void)
{
	#ifdef linux

	int handle, len;
	char buf[256];
	
	printf("\n----- DataLogic TEST --------------------------------------------\n");

	//--- search interface name --------------
	{
		FILE* fp;
		fp = popen("ls /dev/ttyACM*", "r");
		while (!feof(fp))
		{
			fgets(buf, sizeof(buf), fp);
			len = strlen(buf);
			if (len) buf[len-1]=0;
			break;
		}
		pclose(fp);	
	}
	
	_Handle = open(buf, O_RDWR);
	if (_Handle==-1) 
	{
		err_system_error(errno, buf, sizeof(buf));
		printf("Error %d: %s\n", errno, buf);
		return;
	}

	_dl_set_baud(B115200);
	
	//--- parametrize -----------------------------------
	if (FALSE)
	{
		//--- Sevice Mode (RefManual400.pdf p218) ----------------------------------
		_dl_write("$S\n");							// enter service mode

		//--- Reading Parameters (RefManual400.pdf p224) ---------------------------
		_dl_write("$CSNRM01\n");					// Operating Mode: Serial On Line
		_dl_write("$CSPIL02\n");					// Illumination: Endabled
		_dl_write("$CSPTO00\n");					// Phase Off Event: Trigger Stop		
		_dl_write_str("$CSTON", "ON",  20);			// 
		_dl_write_str("$CSTOF", "OFF", 20);
		_dl_write_str("$CLFPR", "Reader01:", 20);	// prefix (each reader different!)
		
		//--- LED and Beeper configuraion (RefManual400.pdf p228) -------------------		
		_dl_write("$CBPPU00\n");					// Power Up Beep:  Disable
		_dl_write("$CBPVO00\n");					// Good Read Beep: Disable
//		_dl_write("$CLSSP00\n");					// Green Spot Duration: Disable
		
		_dl_write("$Ar\n");							// Save and exit service mode
	}	
	
	write(_Handle, "ON", 2);
	_dl_read();
	write(_Handle, "OFF", 3); // now the interface is down!
	close(handle);
	printf("-------------------------------------------------------------\n");

	#endif
}

//--- _dl_write -----------------------------------------------
static void _dl_write(char *str)
{
	int len;
	char buf[256];

	strcpy(buf, str);
	buf[strlen(buf)-1]=0;
	printf("Request: >>%s<<\n", buf);
	len = strlen(str);
	len = write(_Handle, str, len);
	if (len<0) 
	{
		err_system_error(errno, buf, sizeof(buf));
		printf("Error %d: %s\n", errno, buf);
	}
	_dl_read();
}

//--- _dl_write_str --------------------------------------------
static void _dl_write_str(char *str, char *par, int parlen)
{
	char buf[256];
	int len, i;
	
	len =strlen(str);
	strcpy(buf, str);
	for(i=0; i<parlen; i++)
	{
		if (*par) len += sprintf(&buf[len], "%02X", (UCHAR)*par++);
		else      len += sprintf(&buf[len], "00");
	}
	sprintf(&buf[len], "\n");
	_dl_write(buf);
}

//--- _dl_read ----------------------------------
static void  _dl_read(void)
{
	int	len=0;
	char buf[256];
	printf("Response:\n");
	while (len == 0)
	{
		memset(buf, 0, sizeof(buf));
		len=read(_Handle, buf, sizeof(buf));
		if (len>0) printf("\t%s", buf);				
	}
}

//--- _dl_set_baud --------------------
static void _dl_set_baud(int baud)
{
#ifdef linux
	struct termios options;
		
	tcgetattr(_Handle, &options);
	cfsetispeed(&options, baud);
	cfsetospeed(&options, baud);
	
	//mode: 8n1
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag |= CREAD;
	
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	
	tcsetattr(_Handle, TCSANOW, &options);
#endif
}