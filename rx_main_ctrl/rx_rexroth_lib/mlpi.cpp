// RX-Rexroth.cpp : Defines the exported functions for the DLL application.
//
#ifdef linux
	#include <stdarg.h>
	#include <wchar.h>
#else
	#include "stdafx.h"
#endif

#include "rx_common.h"

#include <mlpiGlobal.h>
#include <mlpiApiLib.h>
#include <mlpiLogicLib.h>

#include "rx_error.h"
#include "rx_threads.h"
#include "rx_xml.h"
#include "rx_rexroth.h"
#include "tinyxml.h"
#include "lc.h"
#include "par.h"
#include "mlpi.h"


//--- Global variables -----------------------------
MLPIHANDLE	_Connection = MLPI_INVALIDHANDLE;
static char	_ErrMsg[128]="";
char		_UsrMsg[128]="";
char		_IpAddr[64]="";
wchar_t		_WIpAddr[64]=L"";
wchar_t		_WIpAddr1[64]=L"";
wchar_t		_WApplication[64]=L"";

void (*OnConnected)  (void) = NULL;
void (*OnDeconnected)(void) = NULL;
void (*OnError)      (void) = NULL;


//--- rex_connect -----------------------------------------------------
int  rex_connect(const char *ipAddr, connected_callback onConnected,  connected_callback onDeconnected, error_callback onError)
{
	MLPIRESULT result;
	ULONG cnt;

	err_init(TRUE, 100);

	if (ipAddr)			strcpy(_IpAddr, ipAddr);
//	if (ipAddr)			sprintf(_IpAddr, "%s -user=indraworks", ipAddr);
	if (onConnected)	OnConnected   = onConnected;
	if (onDeconnected)	OnDeconnected = onDeconnected;
	if (onError)		OnError		  =	onError;
	
	if (_Connection != MLPI_INVALIDHANDLE) return 0;

	char_to_wchar(_IpAddr, _WIpAddr, SIZEOF(_WIpAddr));
	result=mlpiApiSetDefaultTimeout(MLPI_TIMEOUT);
	if (FALSE)
	{
		Error(WARN, 0, "Not connect PLC");
		result =  MLPI_E_CONNECTFAILED;
	}
	else result = mlpiApiConnect(_WIpAddr, &_Connection);
	if (rex_check_result(result)) return 1;
	result=mlpiLogicGetNumberOfApplications(_Connection, &cnt);
	for (int i=0; i<10; i++)
	{
		result=mlpiLogicGetNameOfApplication(_Connection, 0, _WApplication, SIZEOF(_WApplication));
		if (*_WApplication) break;
		rx_sleep(100);
	}
		
	lc_get_symbols();

	if (OnConnected) OnConnected();

	return 0;
}

//--- rex_is_connected ------------------------
int rex_is_connected(void)
{
	return (_Connection != MLPI_INVALIDHANDLE);
}

//--- rex_save --------------------------------
int rex_save(char *filepath, char *filter)
{
	TiXmlDocument	*pDoc = (TiXmlDocument*)rx_xml_new();
	lc_save(pDoc, filter);			
	par_save(pDoc);
	pDoc->SaveFile(filepath);
	return 0;
}

//--- rex_load -----------------------------------
int rex_load(char *filepath)
{
	TiXmlDocument *pDoc = (TiXmlDocument*)rx_xml_load(filepath);
	lc_load(pDoc);
	par_load(pDoc);
	return 0;
}

//--- rex_message -----------------------------------------------------
void rex_message(const char *format, ...)
{
	va_list args;

	va_start( args, format );
	vsnprintf(_UsrMsg, SIZEOF(_UsrMsg)-1, format, args);
	_UsrMsg[SIZEOF(_UsrMsg)-1]=0;
	va_end(args);

//	if (OnError) OnError();
}

//--- _deconnected --------------------------------------------
static void _deconnected()
{	if (_Connection!=MLPI_INVALIDHANDLE && OnDeconnected) OnDeconnected();
	_Connection = MLPI_INVALIDHANDLE;
}

//--- rex_check_result -----------------------------------------------
int rex_check_result(MLPIRESULT result)
{
	if (MLPI_FAILED(result))
	{
		int len;
		len = sprintf(_ErrMsg, "0x%X: ", (int)result);
		if (result!=MLPI_S_OK)
			len=len;
		switch (result)
		{
			case 0xf03a0010:				strcpy(&_ErrMsg[len], "Check variable in Symbol Configuration"); break;
			case MLPI_S_OK:					strcpy(&_ErrMsg[len], "OK"); break;
			case MLPI_E_FAIL:               strcpy(&_ErrMsg[len], "General error during function call."); break;
			case MLPI_E_NOTSUPPORTED:       strcpy(&_ErrMsg[len], "The given function is not supported yet."); break;
			case MLPI_E_INVALIDARG:         strcpy(&_ErrMsg[len], "Invalid argument given to method."); break;
			case MLPI_E_OUTOFMEMORY:        strcpy(&_ErrMsg[len], "Out of memory or resources (RAM, sockets, handles, disk space ...)."); break;
			case MLPI_E_TIMEOUT:            strcpy(&_ErrMsg[len], "Timeout during function call.");
											_deconnected();
											break;
			case MLPI_E_SERVEREXCEPTION:    strcpy(&_ErrMsg[len], "System exception on control side."); break;
			case MLPI_E_CONNECTFAILED:      strcpy(&_ErrMsg[len], "Connection failed."); break;
			case MLPI_E_CREATEERROR:        strcpy(&_ErrMsg[len], "Error creating the resource."); break;
			case MLPI_E_SYSTEMERROR:        strcpy(&_ErrMsg[len], "System error during execution."); break;
			case MLPI_E_BUFFERTOOSHORT:     strcpy(&_ErrMsg[len], "Given buffer is too short."); break;
			case MLPI_E_INVALIDSIGNATURE:   strcpy(&_ErrMsg[len], "Invalid signature."); break;
			case MLPI_E_STARTERROR:         strcpy(&_ErrMsg[len], "Error during start of functionality."); break;
			case MLPI_E_WATCHDOGWARNING:    strcpy(&_ErrMsg[len], "Watchdog warning occurred."); break;
			case MLPI_E_WATCHDOGERROR:      strcpy(&_ErrMsg[len], "Watchdog error occurred."); break;
			case MLPI_E_UNIMPLEMENTED:      strcpy(&_ErrMsg[len], "The given function is not implemented on this specific device."); break;
			case MLPI_E_LIMIT_MIN:          strcpy(&_ErrMsg[len], "The minimum of a limitation is exceeded."); break;
			case MLPI_E_LIMIT_MAX:          strcpy(&_ErrMsg[len], "The maximum of a limitation is exceeded."); break;
			case MLPI_E_VERSION:            strcpy(&_ErrMsg[len], "Version conflict."); break;
			case MLPI_E_DEPRECATED:         strcpy(&_ErrMsg[len], "Deprecated. The lib or function is no longer supported."); break;
			case MLPI_E_PERMISSION:         strcpy(&_ErrMsg[len], "Request declined due to missing permission rights."); break;
			case MLPI_E_TYPE_MISSMATCH:     strcpy(&_ErrMsg[len], "Type mismatch, present type doesn't match requested type."); break;
			case MLPI_E_SIZE_MISSMATCH:     strcpy(&_ErrMsg[len], "Size mismatch, present size doesn't match requested size."); break;
			case MLPI_E_INVALID_HANDLE:     strcpy(&_ErrMsg[len], "Invalid handle argument or NULL pointer argument."); 
											_deconnected();
											break;
			case MLPI_E_NOCONNECTION:       strcpy(&_ErrMsg[len], "The connection is not established, no longer established or has been quit by peer."); 
											_deconnected();
											break;
			case MLPI_E_RD_WR_PROTECTION:   strcpy(&_ErrMsg[len], "Request declined due to read or write protection."); break;
		}
	}
	else _ErrMsg[0]=0;
	return MLPI_FAILED(result);
}

//--- mlpi_get_errmsg ---------------
char *mlpi_get_errmsg (void)
{
	return _ErrMsg;
}