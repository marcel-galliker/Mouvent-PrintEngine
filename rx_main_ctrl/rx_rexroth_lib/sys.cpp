// RX-Rexroth.cpp : Defines the exported functions for the DLL application.
//

#ifdef _WIN32
	#include "stdafx.h"
#endif

#include "rx_common.h"

#include <mlpiApiLib.h>
#include <mlpiLogicLib.h>
#include <mlpiSystemLib.h>
#include <mlpiParameterLib.h>
#include <util/timer.h>
#include <util/wchar16.h>
#include <util/mlpiLogicHelper.h>

#include "mlpi.h"
#include "rx_error.h"
#include "rx_xmlDoc.h"
#include "rx_xml.h"
#include "lc.h"
#include "rx_rexroth.h"

static UINT32 _Mode=MLPI_SYSTEMMODE_SWITCHING;

static char _axesName[16][32] = 
	{
		"0",				// 0
		"ScanIn",			// 1
		"ScanOut",			// 2
		"ScanChain",		// 3
		"Belt",				// 4
		"Unwinder",			// 5
		"BufferIn1",		// 6
		"BufferIn2",		// 7
		"PressRollerLeft",	// 8
		"PressRollerRight",	// 9
		"Brushes",			// 10
		"BufferOut1",		// 11
		"BufferOut2",		// 12
		"Rewinder",			// 13
	};

//--- defines ------------------------------------------------


//--- sys_set_axes_name ---------------------------------------
int sys_set_axes_name(int no, char *name)
{
	if (no>=0 && no<SIZEOF(_axesName))
		strncpy(_axesName[no], name, sizeof(_axesName[no]));
	return REPLY_OK;
}

//--- sys_get_display_text -------------------------------------------
int sys_get_display_text (UINT32 *errNo, char *text, int size, UINT32 *pmode)
{
	rex_connect(NULL, NULL, NULL, NULL);

	*errNo = 0;
	*pmode = 0xff;
	strcpy(text, "REXROTH NOT CONNECTED");

	MlpiDiagnosis diagnosis;
	memset(&diagnosis, 0, sizeof(diagnosis));
	MLPIRESULT result = mlpiSystemGetDisplayedDiagnosis(_Connection, &diagnosis);

	if (*_UsrMsg)
	{
		strcpy(text, _UsrMsg);
		if (strlen(_UsrMsg)+2<SIZEOF(_UsrMsg)) strcat(_UsrMsg, ".");
	}
	else if (*mlpi_get_errmsg()) strcpy(text, mlpi_get_errmsg());
	else if (!MLPI_FAILED(result))
	{
		*errNo = diagnosis.number;
		wchar_to_char(diagnosis.text, text, size);
	}

	MlpiSystemMode mode = MLPI_SYSTEMMODE_SWITCHING;
	result = mlpiSystemGetCurrentMode(_Connection, &mode);
	*pmode = (UINT32)mode;
	if (_Mode==MLPI_SYSTEMMODE_SWITCHING && mode!=MLPI_SYSTEMMODE_SWITCHING) 
		*_UsrMsg=0;
	_Mode = mode;

	return rex_check_result(result); 
}

//--- sys_in_run_mode -------------------------------------------
int sys_in_run_mode(void)
{
	rex_connect(NULL, NULL, NULL, NULL);

	MlpiSystemMode mode = MLPI_SYSTEMMODE_SWITCHING;
	MLPIRESULT result = mlpiSystemGetCurrentMode(_Connection, &mode);
	if (rex_check_result(result)==REPLY_OK && mode==MLPI_SYSTEMMODE_BB) return TRUE;
#ifdef DEBUG
	if (rex_check_result(result)==REPLY_OK && mode==MLPI_SYSTEMMODE_P2) return TRUE; 
#endif
	return FALSE;
}

//--- _plclog_to_rxlog ----------------------------------------------
static void _plclog_to_rxlog(int no, MlpiDiagnosis *plc, SPlcLogItem *rx)
{
	char source[64];
	char name[64];
	int len;
	/*
	static int _test=0;
	if (_test==0)
	{	
		_test=1;
		MlpiDiagnosis test;
		printf("state: %ld\n",			(BYTE*)&test.state-				(BYTE*)&test);
		printf("dateTime: %ld\n",		(BYTE*)&test.dateTime-			(BYTE*)&test);
		printf("logicalAddress: %ld\n",	(BYTE*)&test.logicalAddress-	(BYTE*)&test);
		printf("physicalAddress: %ld\n",(BYTE*)&test.physicalAddress-	(BYTE*)&test);
		printf("despatcher: %ld\n",		(BYTE*)&test.despatcher-		(BYTE*)&test);
		printf("index: %ld\n",			(BYTE*)&test.index-				(BYTE*)&test);
		printf("text: %ld\n",			(BYTE*)&test.text-				(BYTE*)&test);
		if ((BYTE*)&test.text-(BYTE*)&test != 44) 
			Error(ERR_CONT, 0, "PLC TYPE SIZE MISMATCH. size=%d, expected=%d", (BYTE*)&test.text-(BYTE*)&test, 44);
		printf("\n");
	}
	*/	
	rx->no    = no;
	rx->errNo = plc->number;
		
	switch (plc->state)
	{
	case MLPI_DIAGNOSIS_PASSIVE:	if ((plc->number & 0xf0000000) == 0xa0000000) rx->state = message;
									else                                          rx->state = passive; 
									break;
	case MLPI_DIAGNOSIS_ACTIVE:		rx->state = active;  break;
	case MLPI_DIAGNOSIS_RESET:		rx->state = reset;   break;
	case MLPI_DIAGNOSIS_UNKNOWN:	rx->state = undef;   break;
	}

	if (plc->logicalAddress < SIZEOF(_axesName)) sprintf(name, "%d(%s)", plc->logicalAddress, _axesName[plc->logicalAddress]);
	else                                         sprintf(name, "%d",plc->logicalAddress);
	
	switch(plc->despatcher)
	{
	case MLPI_DESPATCHER_CONTROL:	sprintf(source, "IndraControl");	break;
	case MLPI_DESPATCHER_DRIVE:		sprintf(source, "Drive %s",	name);	break;
	case MLPI_DESPATCHER_LOGIC:		sprintf(source, "Logic %s",	name);	break;
	case MLPI_DESPATCHER_AXIS:		sprintf(source, "Axis %s",	name);	break;
	case MLPI_DESPATCHER_KINEMATIC:	sprintf(source, "Kinematic (l=0x%08x  p=0x%08x)",	plc->logicalAddress, plc->physicalAddress);	break;
	case MLPI_DESPATCHER_PCI:		sprintf(source, "PCI (l=0x%08x  p=0x%08x)",			plc->logicalAddress, plc->physicalAddress);	break;
	default:						sprintf(source, "(l=0x%08x  p=0x%08x)",				plc->logicalAddress, plc->physicalAddress);	break;
	}
	
	#ifdef WIN32
	TIME_ZONE_INFORMATION tzi;
	SYSTEMTIME plcTime;
	SYSTEMTIME localTime;

	GetTimeZoneInformation(&tzi);
	memset(&plcTime, 0, sizeof(plcTime));
	plcTime.wYear			= plc->dateTime.year;
	plcTime.wMonth			= plc->dateTime.month;
	plcTime.wDay			= plc->dateTime.day;
	plcTime.wHour			= plc->dateTime.hour;
	plcTime.wMinute			= plc->dateTime.minute;
	plcTime.wSecond			= plc->dateTime.second;
	plcTime.wMilliseconds	= plc->dateTime.milliSecond;

	SystemTimeToTzSpecificLocalTime(&tzi, &plcTime, &localTime);

	sprintf(rx->date, "%d.%d.%d  %d:%d:%d.%d", 
		localTime.wDay,  localTime.wMonth, localTime.wYear, 
		localTime.wHour, localTime.wMinute, localTime.wSecond, localTime.wMilliseconds);
	#else
	sprintf(rx->date, "%02d.%02d.%04d  %02d:%02d:%02d.%03d", 
		plc->dateTime.day,  plc->dateTime.month, plc->dateTime.year, 
		plc->dateTime.hour, plc->dateTime.minute, plc->dateTime.second, plc->dateTime.milliSecond);
	#endif
	len=sprintf(rx->text, "%s: ", source);
	wchar_to_char((wchar_t*)plc->text, &rx->text[len], sizeof(rx->text)-len);
//	printf("PLC-LOG: 0x%08x: %s\n", plc->number, rx->text);
}

//--- sys_get_log_item ------------------------------------
int sys_get_log_item(int no, SPlcLogItem *pItem, ULONG *idx)
{
	MlpiDiagnosis diagnosis;
	ULONG numElementsRet = 0;
	ULONG indexNewest = 0;
	ULONG indexOldest = 0;
	memset(&diagnosis, 0, sizeof(diagnosis));
	mlpiSystemGetOldestDiagnosisIndex(_Connection, &indexOldest);
	mlpiSystemGetNewestDiagnosisIndex(_Connection, &indexNewest);
	*idx=-1;

	if (indexNewest-no>indexOldest)
	{
		*idx = indexNewest-no;
		mlpiSystemGetDiagnosisLog(_Connection, indexNewest-no, &diagnosis, 1, &numElementsRet);
		_plclog_to_rxlog(no, &diagnosis, pItem);
		return REPLY_OK;
	}
	return REPLY_ERROR;
}

//--- sys_get_new_log_item -----------------------------------------------
int sys_get_new_log_item(SPlcLogItem *pItem, ULONG *idx)
{
	MlpiDiagnosis diagnosis;
	ULONG indexNewest = 0;
	ULONG numElementsRet = 0;

	mlpiSystemGetNewestDiagnosisIndex(_Connection, &indexNewest);
	if (*idx < indexNewest)
	{
		if (*idx==0) mlpiSystemGetOldestDiagnosisIndex(_Connection, idx);
		else		(*idx)++;
		mlpiSystemGetDiagnosisLog(_Connection, *idx, &diagnosis, 1, &numElementsRet);
		_plclog_to_rxlog(-1, &diagnosis, pItem);
		return REPLY_OK;
	}
	return REPLY_ERROR;
}

//--- sys_get_info ---------------------------------------------
int sys_get_info(SSystemInfo *pinfo)
{
	wchar_t wstr[256];

	memset(pinfo, 0, sizeof(SSystemInfo));

	if (rex_check_result(mlpiSystemGetVersionInfo(_Connection, MLPI_VERSION_HARDWARE, wstr, SIZEOF(wstr)))) return REPLY_ERROR;
	wchar_to_char(wstr, pinfo->versionHardware, SIZEOF(pinfo->versionHardware));

	if (rex_check_result(mlpiSystemGetVersionInfo(_Connection, MLPI_VERSION_FIRMWARE, wstr, SIZEOF(wstr)))) return REPLY_ERROR;
	wchar_to_char(wstr, pinfo->versionFirmware, SIZEOF(pinfo->versionFirmware));

	if (rex_check_result(mlpiSystemGetVersionInfo(_Connection, MLPI_VERSION_LOGIC, wstr, SIZEOF(wstr)))) return REPLY_ERROR;
	wchar_to_char(wstr, pinfo->versionLogic, SIZEOF(pinfo->versionLogic));

	if (rex_check_result(mlpiSystemGetVersionInfo(_Connection, MLPI_VERSION_BSP, wstr, SIZEOF(wstr)))) return REPLY_ERROR;
	wchar_to_char(wstr, pinfo->versionBsp, SIZEOF(pinfo->versionBsp));

	if (rex_check_result(mlpiSystemGetVersionInfo(_Connection, MLPI_VERSION_MLPI_SERVER_CORE, wstr, SIZEOF(wstr)))) return REPLY_ERROR;
	wchar_to_char(wstr, pinfo->versionMlpi, SIZEOF(pinfo->versionMlpi));

	if (rex_check_result(mlpiSystemGetSerialNumber(_Connection, wstr, _countof(wstr)))) return REPLY_ERROR;
	wchar_to_char(wstr, pinfo->serialNo, SIZEOF(pinfo->serialNo));

	if (rex_check_result(mlpiSystemGetHardwareDetails(_Connection, wstr, _countof(wstr)))) return REPLY_ERROR;
	wchar_to_char(wstr, pinfo->hardwareDetails, SIZEOF(pinfo->hardwareDetails));

	if (rex_check_result(mlpiSystemGetLocalBusConfiguration(_Connection, wstr, _countof(wstr)))) return REPLY_ERROR;
	wchar_to_char(wstr, pinfo->localBusConfig, SIZEOF(pinfo->localBusConfig));

	if (rex_check_result(mlpiSystemGetFunctionModulBusConfiguration(_Connection, wstr, _countof(wstr)))) return REPLY_ERROR;
	wchar_to_char(wstr, pinfo->modulBusConfig, SIZEOF(pinfo->modulBusConfig));
	/*
	for (i=0, len=0; TRUE; i++)
	{
		if (rex_check_result(mlpiSystemGetFunctionModuleHardwareDetails(_Connection, i, wstr, _countof(wstr)))) break;
		wchar_to_char(wstr, &pinfo->modulHwDetails[len], SIZEOF(pinfo->modulHwDetails)-len);
		len += (int)wcslen(wstr);
	}
	*/

	if (rex_check_result(mlpiSystemGetTemperature   (_Connection, &pinfo->tempAct))) return REPLY_ERROR;
	if (rex_check_result(mlpiSystemGetTemperatureMax(_Connection, &pinfo->tempMax))) return REPLY_ERROR;

	mlpiSystemGetOldestDiagnosisIndex(_Connection, (ULONG*)&pinfo->indexOldest);
	mlpiSystemGetNewestDiagnosisIndex(_Connection, (ULONG*)&pinfo->indexNewest);

	/*
	MlpiSystemMode mode = MLPI_SYSTEMMODE_P0;
	result = mlpiSystemGetCurrentMode(_Connection, &mode);
	swprintf(str, L"Mode:  %d\n", mode);
	OutputDebugStringW(str);
		
	// Read the date and time of the device.
	MlpiDateAndTime dateTime;
	memset(&dateTime, 0, sizeof(dateTime));
	result = mlpiSystemGetDateAndTimeUtc(_Connection, &dateTime);
	swprintf(str, L"System time: %04u-%02u-%02u %02u-%02u-%02u (UTC)",
			dateTime.year, dateTime.month, dateTime.day,
			dateTime.hour, dateTime.minute, dateTime.second);
	OutputDebugStringW(str);
	*/

	return REPLY_OK;
}

//--- sys_get_time -----------------------------------------
int sys_get_time	 (SSystemTime *ptime)
{
	return rex_check_result(mlpiSystemGetDateAndTimeUtc(_Connection, (MlpiDateAndTime*)ptime));
}

//--- sys_set_time -----------------------------------------
int sys_set_time	 (SSystemTime *ptime)
{
	return rex_check_result(mlpiSystemSetDateAndTimeUtc(_Connection, *(MlpiDateAndTime*)ptime));
}

//--- sys_reset_error ------------------------------------------
int sys_reset_error	 (void)
{
	lc_reset_error();
	mlpiApiSetDefaultTimeout(0);
	return rex_check_result(mlpiSystemClearError(_Connection));
}

//--- sys_set_mode ------------------------------------------
int sys_set_mode (UINT32 mode)
{
	return rex_check_result(mlpiSystemSetTargetMode(_Connection, (MlpiSystemMode)mode));
}

//--- sys_reboot ------------------------------------------
int sys_reboot	 (void)
{
	rex_message("Booting");

	sys_set_mode(0);
	MLPIRESULT result = mlpiSystemReboot(_Connection);
	mlpiApiDisconnect (&_Connection);

	return rex_check_result(result);
}

//--- sys_clean_up ------------------------------------------
int sys_clean_up	 (void)
{
	rex_message("Booting");
	MLPIRESULT result = mlpiSystemCleanup(_Connection);
	mlpiApiDisconnect (&_Connection);
	return rex_check_result(result);
}