// ****************************************************************************
//
//	DIGITAL PRINTING - rx_error.c
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include <stdarg.h>
#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_log.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"

#ifdef WIN32
#pragma warning( disable: 4996 )
#endif

//--- module global variables -----------------------------------------------------------------
static int					sInit = FALSE;
static HANDLE				sMutex = NULL;
static int					sErrorCnt=0;
static ELogItemType			sErrorType;
static char					sErrorStr[256];
static char					sErrorStrTemp[256];
static int					sListSize=0;
static SLogItem				*sList=NULL;
static log_Handle			sLogHdl  = NULL;
static HANDLE				shServer = NULL;
static RX_SOCKET			sClient  = 0;

static EDevice				sDevType=dev_undef;
static int					sDevNo=-1;

static ErrorHandler _OnError = NULL;

//--- prototypes ---------------------------------------------------------------------------

// static int  setDeviceStr(char *str, EDevice deviceType, int deviceNo);
void compose_message(EDevice deviceType, int deviceNo, int errNo, char *str, int strSize, const char *format, const BYTE *arg);
// static int	errExists(EDevice deviceType, int deviceNo, int errNo);


//--- Constructor ---------------------------------------------------------------------
void err_init(int log, int listSize)
{
	if (!sInit)
	{
		if (!sMutex)	sMutex = rx_mutex_create();
		sInit = TRUE;
		sErrorCnt  = 0;
		shServer = NULL;
		sClient  = 0;
		sListSize = listSize;
		if (sList) free(sList);
		if (sListSize) sList = (SLogItem*)malloc(sListSize*sizeof(SLogItem));
		if (sList) memset(sList, 0, sListSize*sizeof(SLogItem));

		if (log) log_open(PATH_LOG FILENAME_LOG, &sLogHdl, TRUE);
	}
}

//--- Destructor ----------------------------------------------------------------------
void err_end()
{
	if (sInit)
	{
		log_close(&sLogHdl);
		if (sList) free(sList);
		sList = 0;
		sListSize = 0;
		sErrorCnt = 0;
#ifdef WIN32
		CloseHandle(sMutex);
#endif
	}
}

//--- err_SetHandler --------------------------------------------
void err_SetHandler(ErrorHandler handler)
{
	_OnError = handler;
}

//--- err_set_device ------------------------------------
void err_set_device(EDevice device, int no)
{
	sDevType  = device;
	sDevNo	  = no;
}

//--- err_set_server ------------------------------------
void err_set_server(HANDLE hServer)
{
	shServer = hServer;
}

//--- err_set_client ------------------------------------
void err_set_client(RX_SOCKET socket)
{
	sClient = socket;
}

//--- err_get_state -----------------------------------------------------------------------
void err_get_state(ELogItemType *pState, char *text, int size)
{
	*pState = sErrorType;
	if (text) strcpy(text, sErrorStr);
}

//--- err_clear_all ------------------------------------------------------------------------
void err_clear_all(void)
{
	rx_mutex_lock(sMutex);

	memset(sList, 0, sListSize*sizeof(SLogItem));
	sErrorCnt = 0;
	sErrorType = LOG_TYPE_LOG;
	memset(sErrorStr, 0, sizeof(sErrorStr));

	rx_mutex_unlock(sMutex);

	if (_OnError) _OnError(LOG_TYPE_UNDEF, "", 0, "");
}

//--- add_to_list -------------------------
static void add_to_list(SLogItem *pErr)
{
	if (sErrorCnt < sListSize)
	{
		memcpy(&sList[sErrorCnt], pErr, sizeof(SLogItem));
		if (sErrorCnt+1 < sListSize) sErrorCnt++;
	}
}

//--- err_get_log_item ------------------------------------------
SLogItem  *err_get_log_item(int no)
{
	if (no < sErrorCnt) return &sList[no];
	else return NULL;
}

//--- err_system_error ------------------------------------------
char *err_system_error(int no, char *buffer, int size)
{
#ifdef WIN32
	int i;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		buffer,
		size, NULL);
	for (i=(int)strlen(buffer)-1; i>=0 && buffer[i]<' '; i--)
		buffer[i] = 0;
	return buffer;
#else
	strncpy(buffer, strerror(no), size);
	return buffer;
#endif
}

//--- errExists --------------------------------------------
/*
static int errExists(EDevice deviceType, int deviceNo, int errNo)
{
	int i;
	for (i=0; i<m_ErrorCnt; i++)
	{
		if (m_List[i].deviceType == deviceType && m_List[i].deviceNo == deviceNo && m_List[i].errNo == errNo) return TRUE;
	}
	return FALSE;
}
*/

/*
//--- GetMessageStr ------------------------------------------------------------------
void err_get_message_str(char *str, int strSize, int errNo)
{
	wchar_t err[20];

	sprintf(err, "%d", errNo);
	Language.GetText((wchar_t*)str, strSize, m_chapter, err);
}
*/

//--- setDeviceStr --------------------------------------------------------------------
/*
static int setDeviceStr(char *str, EDevice deviceType, int deviceNo)
{
	if (deviceType>dev_undef && deviceType<dev_end)
	{
		return sprintf(str, "%s: ", DeviceStr[deviceType]);
	}
	else *str=0;
	return 0;
}
*/


//--- compose_message ---------------------------------------------------------
void compose_message(EDevice deviceType, int deviceNo, int errNo, char *str, int strSize, const char *format, const BYTE *arg)
{
	const char	*ch;
	char	*res	= str;
	char	*fs;
	int		l;
	char	stringi[50];

	USHORT	arg_c[10];
	int		arg_d[10];
	char	*arg_s[10];
	int		c, d, s;

	memset(arg_c, 0, sizeof(arg_c));
	memset(arg_d, 0, sizeof(arg_d));
	memset(arg_s, 0, sizeof(arg_s));
	c=d=s=0;
	//--- scan arguments ---
	while (TRUE)
	{
		if (*arg=='c')
		{
			memcpy(&arg_c[c++], ++arg,  1);
			arg += 1;
		}
		else if (*arg=='d')
		{
			memcpy(&arg_d[d++], ++arg, sizeof(int));
			arg += sizeof(int);
		}
		else if (*arg=='s')
		{
			arg_s[s++] = (char*)++arg;
			l = (int)strlen((char*)arg);
			arg += l+1;
		}
		else break;
	}
	//--- end scan ---

	ch = format;
	l = 0;//setDeviceStr(res, deviceType, deviceNo);
	res+=l;
	strSize-=l;
	c=d=s=0;
	while(*ch && strSize>0){
		if (*ch == L'%'){
			fs = stringi;
			*fs++ = *ch++;	// '%'
			*fs   = *ch++;
label:		switch (*fs){

			case L'%':
			case 0x00:	*res=0;
						return; // end of line reached

			case L'c':
			case L'C':	if (*ch>=L'1' && *ch<=L'9')
						{
							c = *ch++-L'1';
						}
						memcpy(res, &arg_c[c++], 2);
						strSize-=2;
						res ++;
						break;

			case L'd':
			case L'i':
			case L'o':
			case L'u':
			case L'x':
			case L'X':	if (*ch>=L'1' && *ch<=L'9')
						{
							d = *ch++-L'1';
						}
						*++fs = 0;
						sprintf(res, stringi, arg_d[d++]);
						strSize-=(int)strlen(res);
						res += strlen(res);
						break;

			case L's':
			case L'S':	if (*ch>='1' && *ch<='9')
						{
							s = *ch++-'1';
						}
						*++fs = 0;
						if (arg_s[s]!=NULL)
						{
							l = (int)strlen((char*)arg_s[s]);
							memcpy(res, arg_s[s++], l);
							res[l]=0;
							strSize-=l;
							res += l;
						}
						break;

			default:	*(++fs) = *ch++;
						goto label;
			}
		}
		else
		{
			*res++ = *ch++;
			strSize--;
		}
	}
	*res=0;
}

//--- error ------------------------------------------------------------------------
static int error(EDevice device, int no, ELogItemType type, const char *file, int line, int errNo, const char *format, void *parg, va_list arglist)
{
	SLogItem	log;
	int	arg, l, val;
	const char	*ch;
	char	*f, *start;
	char	*str;
	char	deviceStr[32];

	if (!sInit)
	{
		printf("ERROR not initialized\n");
		return REPLY_ERROR;
	};

	rx_mutex_lock(sMutex);

//	if (GT_Network.mySlaveNo==0) send=TRUE;

	memset(&log, 0, sizeof(log));

	rx_get_system_time(&log.time);

	log.logType= type;
	log.errNo = errNo;
	log.deviceType = device;
	log.deviceNo = no;

	log.line = line;
	start = (char*)file;
	for (f=(char*)file; *f; f++)
	{
		if (*f=='\\') start=f+1;
	}
	strncpy(log.file, start, sizeof(log.file));
	strncpy(log.formatStr, format,sizeof(log.formatStr));

	if (parg) 
	{
		memcpy(log.arg, parg, sizeof(log.arg));
	}
	else
	{
		//--- save arguments ---
		ch = format;
		arg = 0;

		while(*ch){
			if (*ch++ == L'%'){
	label1:
				switch (*ch++){
				case 0x00:	break; // end of line reached
				case L'%':  break;

				case L'c':
				case L'C':	log.arg[arg++] = 'c';
							#ifdef linux
								val = va_arg(arglist, int);
							#else
								val = va_arg(arglist, char);
							#endif
							*(char*)&log.arg[arg] = (char)val;
							arg += 1;
							break;
				case L'd':
				case L'i':
		 		case L'o':
				case L'u':
				case L'x':
				case L'X':	log.arg[arg++] = 'd';
							val = va_arg(arglist, int);
							memcpy(&log.arg[arg], &val, sizeof(int));
							arg += sizeof(int);
							break;
				case L's':
				case L'S':	log.arg[arg++] = 's';
							str = va_arg(arglist, char*);
							l=(int)strlen(str);
							memcpy(&log.arg[arg], str, l);
							log.arg[arg + l] = 0;
							arg += l+1;
							break;

				default:	goto label1;
				}
			}
		}
	}

	add_to_list(&log);

	if (sLogHdl) log_add_item(sLogHdl, &log);

	{
		char timestr[64];
		rx_get_system_time_str(timestr, '.');
		compose_message((EDevice)log.deviceType, log.deviceNo, errNo, sErrorStrTemp, SIZEOF(sErrorStrTemp), (const char*)format, log.arg);
		if (device==dev_undef) *deviceStr=0;
		else sprintf(deviceStr, "%s %d", DeviceStr[device], no);
		if (device==dev_undef)	TrPrintfL(1, "{%s} %s:%s [%s:%d]", 							   timestr, LogItemTypeStr[type], sErrorStrTemp, log.file, log.line);
		else					TrPrintfL(1, "%s[%d] {%s} %s: %s [%s:%d]", DeviceStr[device], no, timestr, LogItemTypeStr[type], sErrorStrTemp, log.file, log.line);
	}

	if (type>sErrorType || (type==sErrorType && sErrorType<LOG_TYPE_ERROR_CONT))
	{
		memcpy(sErrorStr, sErrorStrTemp, sizeof(sErrorStr));
	}

	if (type > sErrorType) sErrorType = type;

	rx_mutex_unlock(sMutex);
		
	if (shServer) sok_send_to_clients_2(shServer, NULL, EVT_GET_EVT, sizeof(log), &log);
	if (sClient)  sok_send_2(&sClient, EVT_GET_EVT, sizeof(log), &log);
	
	if (_OnError) _OnError(sErrorType, deviceStr, no, sErrorStrTemp);
	
	if (errNo) return errNo;
	else return REPLY_ERROR; 
}

//--- Error ------------------------------------------------------------------------
int Error(ELogItemType type, const char *file, int line, int errNo, const char *format, ...)
{
	va_list arglist;
	int ret;

	va_start(arglist, format);
	ret=error(sDevType, sDevNo, type, file, line, errNo, format, NULL, arglist);
	va_end(arglist);
	return ret;
}

//--- ErrorEx ------------------------------------------------------------------------
int ErrorEx(EDevice device, int no, ELogItemType type, const char *file, int line, int errNo, const char *format, ...)
{
	va_list arglist;
	int ret;

	va_start(arglist, format);
	ret=error(device, no, type, file, line, errNo, format, NULL, arglist);
	va_end(arglist);
	return ret;
}

//--- ErrorFlag ----------------------------------------------------------------------------------
int  ErrorFlag(ELogItemType type, const char *file, int line, UINT32 *flags, UINT32 flag, int errNo, const char *format, ...)
{
	int ret = REPLY_OK;
	if (!((*flags) & flag))
	{
		va_list arglist;
		va_start(arglist, format);
		ret=error(sDevType, sDevNo, type, file, line, errNo, format, NULL, arglist);
		va_end(arglist);			
		(*flags) |= flag; 
	}
	return ret;
}

//--- ErrorExFlag ----------------------------------------------------------------------------------
int  ErrorExFlag(EDevice device, int no, ELogItemType type, const char *file, int line, UINT32 *flags, UINT32 flag, int errNo, const char *format, ...)
{
	int ret = REPLY_OK;
	if (!((*flags) & flag))
	{
		va_list arglist;
		va_start(arglist, format);
		ret=error(device, no, type, file, line, errNo, format, NULL, arglist);
		va_end(arglist);			
		(*flags) |= flag; 
	}
	return ret;
}

//--- SlaveError --------------------------------------------------------------
int SlaveError(EDevice device, int no, SLogItem *log)
{
	if (log->deviceType!=dev_undef)
	{ 
		device=log->deviceType;
		no    =log->deviceNo;
	}
#ifdef linux
	va_list arglist;
	return error(device, no, (ELogItemType)log->logType, log->file, log->line, log->errNo, log->formatStr, log->arg, arglist);
#else
	return error(device, no, (ELogItemType)log->logType, log->file, log->line, log->errNo, log->formatStr, log->arg, NULL);
#endif
}
