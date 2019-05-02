/******************************************************************************/
/** rx_ink_common.h
 **
 ** Communication between NIOS and Conditioner
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/

#ifndef __RX_INK_COMMON__
#define __RX_INK_COMMON__

#if defined(NIOS)
	#include <alt_types.h>
	#define UCHAR	alt_u8
	#define BYTE	alt_u8
	#define UINT8	alt_u8
	#define UINT16	alt_u16
	#define INT16	alt_16
	#define UINT32	alt_u32
	#define INT32	alt_32
	#define UINT64	alt_u64
	#define INT64	alt_64

	#define REPLY_OK 0
	#define REPLY_ERROR 1

	#define RX_TYPES
#elif defined(COND)

	#include <stdint.h>

	#define UCHAR	uint8_t
	#define BYTE	uint8_t
    #define UINT8	uint8_t
	#define UINT16	uint16_t
	#define INT16	int16_t
	#define UINT32	uint32_t
	#define INT32	int32_t
	#define UINT64	uint64_t 
	#define INT64	int64_t

	#define RX_TYPES
#else
	#include "../rx_common_lib/rx_common.h"
#endif

#ifndef SIZEOF
#define SIZEOF(x) (sizeof((x)) / sizeof((*x)))
#endif

// clang-format off
#ifndef HALT
#define HALT()              \
do {                        \
    while (1) {}            \
} while(0)
#endif


/// If `assertion_test` is false, halt in an endless loop for further
/// investigation
#ifndef ASSERT
#define ASSERT(assert_test)                                            \
do {                                                                   \
	if (!(assert_test))                                                \
	{                                                                  \
		DBG_PRINTF("ASSERT (%s) FAILED in file \"%s\" on line \"%d\" " \
				   "function \"%s\"\n",                                \
				   #assert_test, __FILE__, __LINE__, __func__);        \
		HALT();                                                        \
	}                                                                  \
} while (0)
#endif
// clang-format on


/*
 * Define Boolean type
 * Let everybody know that certain variables are only 1 bit
 */

#ifdef COND
	#ifndef BOOL
		#define BOOL _Bool
	#endif
#endif

// Union to acces bytes of a 32bit variable directly
typedef union {
    UINT32 value;
    BYTE byte[4];
} UINT32u;

typedef union {
    INT32 value;
    BYTE byte[4];
} INT32u;

//#ifdef RX_TYPES
typedef struct SVersion
{
    UINT32 major;    // Year
    UINT32 minor;    // Quarter
    UINT32 revision; // SVN-Revision number
    UINT32 build;    // incremented by each compilation
} SVersion;
//#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define INVALID_VALUE  0x5555aaaa
#define VAL_UNDERFLOW  0x5555aaab
#define VAL_OVERFLOW   0x5555aaac

#define CALIBRATE_PUMP_SPEED    30
#define INK_PRESSURE_MAX        1200

//--- printer ------------------------------------
typedef enum EPrinterType
{
	printer_undef,				// 0: not defined
	printer_test_table,			// 1:
	printer_test_slide,			// 2:
	printer_test_slide_only,	// 3:

	//--- web printers ------------------------------
	printer_LB701=1000,			// 1000: 
	printer_LB702_UV,			// 1001:
	printer_LB702_WB,			// 1002:
	
	printer_DP803=1100,			// 1100:
	 
	//--- scanning printers --------------------------
	printer_TX801     = 2000,		// 2000: Fashionn stanrard output
	printer_TX802,					// 2001: Fashion high outpput

	//--- special projects ----------------
	printer_cleaf = 10000,		// 10000: Cleaf machine
} EPrinterType;

//--- EnFluidCtrlMode -----------------------------
typedef enum EnFluidCtrlMode
{
    ctrl_undef,          // 0x000:
	ctrl_shutdown,	     //	0x001:
//	ctrl_shutdown_done,	 //	0x002:
	ctrl_error		= 0x002,
    ctrl_off		= 0x003,            //	0x003:
    ctrl_warmup,         // 0x004:
    ctrl_readyToPrint,   // 0x005:
    ctrl_print,          // 0x006:
    ctrl_bleed,          // 0x007:
    ctrl_08,             // 0x008:
    ctrl_09,        	 // 0x009:
    ctrl_0a,      		 //	0x00a:
    ctrl_0b, 		     // 0x00b:
    ctrl_flush_night,	 //	0x00c:
    ctrl_flush_weekend,	 //	0x00d:
    ctrl_flush_week,	 //	0x00e:	
	ctrl_flush_step1,	 // 0x00f:
	ctrl_flush_step2,	 // 0x010:
    ctrl_flush_done,     //	0x011:

    ctrl_purge_soft = 0x100,  // 0x100:
    ctrl_purge,               // 0x101:
	ctrl_purge_hard,          // 0x102:
	ctrl_purge_micro,		  // 0x103:
    ctrl_purge_step1 = 0x111, // 0x111:
    ctrl_purge_step2,         // 0x112:
    ctrl_purge_step3,         // 0x113:
	
	ctrl_wipe			= 0x120,  // 0x120:
	ctrl_wetwipe		= 0x121,  
	ctrl_wash			= 0x122,  
	ctrl_wipe_step1		= 0x131,  
	ctrl_wipe_step2		= 0x132,  
	ctrl_wipe_step3		= 0x133,  
	
	ctrl_cap			= 0x140,
	ctrl_cap_step1		= 0x141,
	ctrl_cap_step2		= 0x142,
	ctrl_cap_step3		= 0x143,
	ctrl_cap_step4		= 0x144,	
	
    ctrl_fill = 0x200, //	0x200:
    ctrl_fill_step1,   //	0x201:
    ctrl_fill_step2,   //	0x202:
    ctrl_fill_step3,   //	0x203:
    ctrl_fill_step4,   //	0x204:
    ctrl_fill_step5,   //	0x205:

    ctrl_empty = 0x300, //	0x300:
    ctrl_empty_step1,   //	0x301:
    ctrl_empty_step2,   //	0x302:
    ctrl_empty_step3,   //	0x303:
    ctrl_empty_step4,   //	0x304:
    ctrl_empty_step5,   //	0x305:
	
	ctrl_cal_start = 0x400, // 0x400: 
	ctrl_cal_step1,    		// 0x401: Evaluate PID-Offset to pump 40ml/min
    ctrl_cal_step2,  		// 0x402:
    ctrl_cal_step3,   		// 0x403:
    ctrl_cal_step4,   		// 0x404:
	ctrl_cal_done,       	// 0x405:

    ctrl_test_watchdog = 0x10000,
    ctrl_test, 				// 0x10001
	ctrl_offset_cal, 		// 0x10002
	ctrl_offset_cal_done,	// 
} EnFluidCtrlMode;

char *FluidCtrlModeStr(EnFluidCtrlMode mode);

typedef enum EnDownloadState
{
    dnl_undef,
    dnl_start,
    dnl_prepare,
    dnl_reset,
    dnl_send_ram_kernel,
    dnl_start_ram_kernel,
    dnl_read_flash,
    dnl_chip_erase,
    dnl_blank_check,
    dnl_write_binary,
    dnl_mcu_reset2,
    dnl_done,
    dnl_ok,
} EnDownloadState;

char *DownloadStateStr(EnDownloadState mode);

int valid(int val);

char *value_str(int val);
char *value_str1(int val);
char *value_str3(int val);
char *value_str_temp(int val);
char *value_str_u(int val);
//INT32 convert_i16_to_i32(INT16 val);

#endif // __RX_INK_COMMON__
