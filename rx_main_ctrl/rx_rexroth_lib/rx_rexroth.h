// ****************************************************************************
//
//	
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#ifdef __cplusplus
extern "C"{
#endif

#include "rx_common.h"

typedef enum
{
	CPU,		// 0
	Unwinder,	// 1
	Infeed,		// 2
	Outfeed,	// 3
	Rewinder,	// 4
//	dev_5,
//	dev_6,
//	dev_7,
	PressRoller_2010001=8,	// 8
	PressRoller_2010002=9,	// 9
	MasterAxes=16
} Eplc_Device;

typedef struct SSystemInfo
{
	char	versionHardware[32];
	char	versionFirmware[32];
	char	versionLogic[32];
	char	versionBsp[32];
	char	versionMlpi[32];		// MLPI server core
	char	serialNo[32];
	char	hardwareDetails[256];
	char	localBusConfig[256];
	char	modulBusConfig[256];
	char	modulHwDetails[256];
	float	tempAct;
	float	tempMax;
	UINT32	indexOldest;
	UINT32	indexNewest;
} SSystemInfo;

typedef struct SSystemTime
{
  INT16  year;           //! Year, 2000..2099
  INT16  month;          //! Month, 1..12: 1==January, ..., 12==December
  INT16  day;            //! Day of month, 1..31
  INT16  hour;           //! Hour after midnight, 0..23
  INT16  minute;         //! Minute after hour, 0..59
  INT16  second;         //! Seconds after minute, 0..59
  INT16  milliSecond;    //! Milliseconds after second, 0..999
  INT16  microSecond;    //! Microseconds after millisecond, 0..999
  INT16  dayOfWeek;      //! Day of Week, 1..7: 1==Monday, ..., 7==Sunday
  INT16  dayOfYear;      //! Day of Year, 1..366: 1==January 1, 365/366== December 31
}SSystemTime;

typedef enum
{
	undef	  = 0x00,
	passive   = 0x00,   //!< diagnosis entry is passive.
	active    = 0x01,   //!< diagnosis entry is active.
	reset     = 0x02,   //!< diagnosis entry is reset.
	message   = 0x04,
} EnPlcLogState;

typedef struct SPlcLogItem
{
	int no;
	EnPlcLogState state;
	UINT32 errNo;
	char date[32];
	char text[256];
} SPlcLogItem;

typedef void (*connected_callback)	(void);
typedef void (*error_callback)		(void);

DLL_EXPORT char *mlpi_get_errmsg (void);
DLL_EXPORT int rex_connect       (const char *ipAddr, connected_callback OnConnected, connected_callback OnDeconnected, error_callback OnError);
DLL_EXPORT int rex_is_connected	 (void);

DLL_EXPORT int rex_save		     (char *filepath, char *filter);
DLL_EXPORT int rex_load		     (char *filepath);
DLL_EXPORT void rex_set_simu	 (int simu);


DLL_EXPORT int lc_get_var					(int no, char *name, char *value);
DLL_EXPORT int lc_get_value					(int no, char *value);
DLL_EXPORT int lc_get_value_all				(char *valList, int size);	
DLL_EXPORT int lc_get_value_by_name			(char *name, char *value);
DLL_EXPORT int lc_get_value_by_name_UINT32	(char *name, UINT32 *pvalue);
DLL_EXPORT int lc_get_value_by_name_FLOAT	(char *name, FLOAT *pvalue);
DLL_EXPORT int lc_set_value_by_name			(const char *name, char *value);
DLL_EXPORT int lc_set_value_by_name_UINT32	(const char *name, UINT32 value);
DLL_EXPORT int lc_set_value_by_name_FLOAT	(const char *name, double value);

DLL_EXPORT int par_get_name		(int no, char *device, char *sidn, char *name, char *value, int size);
DLL_EXPORT int par_get_value	(const char *sidn, char *value, int size);
DLL_EXPORT int par_set_value	(const char *sidn, char *value);

DLL_EXPORT int sys_set_axes_name(int no, char *name);
DLL_EXPORT int sys_get_display_text (UINT32 *errNo, char *text, int size, UINT32 *mode);
DLL_EXPORT int sys_in_run_mode      (void);
DLL_EXPORT int sys_get_log_item		(int no, SPlcLogItem *pItem, ULONG *idx);
DLL_EXPORT int sys_get_new_log_item	(SPlcLogItem *pItem, ULONG *idx);
DLL_EXPORT int sys_get_info			(SSystemInfo *pinfo);
DLL_EXPORT int sys_get_time			(SSystemTime *ptime);
DLL_EXPORT int sys_set_time			(SSystemTime *ptime);

DLL_EXPORT int sys_reset_error	 (void);
DLL_EXPORT int sys_set_mode		 (UINT32 mode);
DLL_EXPORT int sys_reboot		 (void);
DLL_EXPORT int sys_clean_up		 (void);

DLL_EXPORT int app_start		 (void);
DLL_EXPORT int app_stop			 (void);
DLL_EXPORT int app_reset		 (void);
DLL_EXPORT int app_download		 (const char *filepath);
								// return = 0: file transfered
								//			1: soure file not found
								//          2: file was up to date
DLL_EXPORT int app_reload_user_rights(void);
								
DLL_EXPORT int app_transfer_file(const char *filepath, const char* destSubDir);
								// return = 0: file transfered
								//			1: soure file not found
								//			2: fils already exists
								//			3: transmit error
#ifdef __cplusplus
}
#endif