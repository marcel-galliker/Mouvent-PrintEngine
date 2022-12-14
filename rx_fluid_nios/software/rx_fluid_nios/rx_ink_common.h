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
	
	#define REPLY_OK	0
	#define REPLY_ERROR	1
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
    printer_undef,           // 0: not defined
    printer_test_table,      // 1:
    printer_test_slide,      // 2:
    printer_test_slide_only, // 3:
    printer_test_table_seon, // 4:
    printer_test_slide_HB,   // 5:
    printer_test_CTC,		 // 6: CTC=Conditioner Test Center

    //--- web printers ------------------------------
    printer_LB701 = 1000, // 1000:
    printer_LB702_UV,     // 1001:
    printer_LB702_WB,     // 1002:
    printer_LH702,        // 1003:

    printer_DP803 = 1100, // 1100:
    printer_setup_assist = 1999, // 1999:

    //--- scanning printers --------------------------
    printer_TX801 = 2000, // 2000: Fashionn stanrard output
    printer_TX802,        // 2001: Fashion high outpput
    printer_TX404,        // 2002:

    printer_CB612 = 3001,
    
    printer_Dropwatcher = 4000,			// 4000:

	//--- special projects ----------------
	printer_cleaf = 10000,		// 10000: Cleaf machine

} EPrinterType;

typedef enum EEncoderType
{
    enc_Balluff,
    enc_Renishaw,
}
EEncoderType;

//--- EnFluidCtrlMode -----------------------------
typedef enum EnFluidCtrlMode
{
    ctrl_undef,          // 0x000:
	ctrl_01,			 //	0x001:
	ctrl_02,			 // 0x002:
	ctrl_error,			 // 0x003:
    ctrl_wait,			 // 0x004:
    ctrl_off,			 // 0x005:           
    ctrl_warmup,         // 0x006:
    ctrl_readyToPrint,   // 0x007:
	ctrl_prepareToPrint, // 0x008:
	ctrl_print,          // 0x009:
    
	
	ctrl_check_step0		= 0x010,    // 0x010:
	ctrl_check_step1,			// 0x011
	ctrl_check_step2,			// 0x012
	ctrl_check_step3,			// 0x013	
	ctrl_check_step4,			// 0x014
	ctrl_check_step5,			// 0x015
	ctrl_check_step6,			// 0x016
	ctrl_check_step7,			// 0x017
	ctrl_check_step8,			// 0x018
	ctrl_check_step9,			// 0x019
	
	ctrl_shutdown		= 0x040,// 0x040:	send slide to cap position
	ctrl_shutdown_step1,		// 0x041:	wait until slide in cap
	ctrl_shutdown_step2,		// 0x042:	start pressure reduction
	ctrl_shutdown_step3,		// 0x043:	wait until pressure low

	ctrl_flush_night = 0x080,	// 0x080:
    ctrl_flush_weekend,	 //	0x081:
    ctrl_flush_week,	 //	0x082:	
	ctrl_flush_step1,	 // 0x083:
	ctrl_flush_step2,	 // 0x084:
	ctrl_flush_step3,	 // 0x085:
	ctrl_flush_step4,	 // 0x086:
    ctrl_flush_done,     //	0x087:

    ctrl_purge_soft = 0x100,  // 0x100:
    ctrl_purge,               // 0x101:
	ctrl_purge_hard,          // 0x102:
	ctrl_purge_hard_wipe,	  // 0x103:
	ctrl_purge_hard_vacc,	  // 0x104:
    ctrl_purge_hard_wash,	  // 0x105:
    ctrl_purge4ever,		  // 0x106:
    ctrl_purge_step1 = 0x111, // 0x111:
    ctrl_purge_step2,         // 0x112:
    ctrl_purge_step3,         // 0x113:
    ctrl_purge_step4,         // 0x114:
    ctrl_purge_step5,         // 0x115:
    ctrl_purge_step6,         // 0x116:
	
	ctrl_wipe			= 0x120,  // 0x120:
	ctrl_wipe_step1		= 0x131,  // 0x131:
	ctrl_wipe_step2		= 0x132,  // 0x132:
	ctrl_wipe_step3		= 0x133,  // 0x133:
	ctrl_wipe_step4		= 0x134,  // 0x134:
	ctrl_wipe_step5		= 0x135,  // 0x135:
	ctrl_wipe_step6		= 0x136,  // 0x136:
	
	ctrl_cap			= 0x140,	// 0x140:
	ctrl_cap_step1		= 0x141,	// 0x141:
	ctrl_cap_step2		= 0x142,	// 0x142:
	ctrl_cap_step3		= 0x143,	// 0x143:
	ctrl_cap_step4		= 0x144,	// 0x144:
	ctrl_cap_step5		= 0x145,	// 0x145:
    ctrl_cap_step6		= 0x146,	// 0x146:
	
	ctrl_vacuum			= 0x150,	// 0x150:
	ctrl_vacuum_step1	= 0x151,	// 0x151:
	ctrl_vacuum_step2	= 0x152,	// 0x152:
	ctrl_vacuum_step3	= 0x153,	// 0x153:
	ctrl_vacuum_step4	= 0x154,	// 0x154:
	ctrl_vacuum_step5	= 0x155,	// 0x155:
	ctrl_vacuum_step6	= 0x156,	// 0x156:
	ctrl_vacuum_step7	= 0x157,	// 0x157:
	ctrl_vacuum_step8	= 0x158,	// 0x158:
	ctrl_vacuum_step9	= 0x159,	// 0x159:
	ctrl_vacuum_step10	= 0x15a,	// 0x15a:
	ctrl_vacuum_step11	= 0x15b,	// 0x15b:
	ctrl_vacuum_step12	= 0x15c,	// 0x15c:
	ctrl_vacuum_step13	= 0x15d,	// 0x15d:
	ctrl_vacuum_step14	= 0x15e,	// 0x15e:
	ctrl_vacuum_high	= 0x15f,	// 0x15f:
	
	ctrl_wash			= 0x160,	// 0x160:
	ctrl_wash_step1		= 0x161,  	// 0x161:
	ctrl_wash_step2		= 0x162,  	// 0x162:
	ctrl_wash_step3		= 0x163,  	// 0x163:
	ctrl_wash_step4		= 0x164,  	// 0x164:
	ctrl_wash_step5		= 0x165,  	// 0x165:
	ctrl_wash_step6		= 0x166,  	// 0x166:

    ctrl_fill = 0x200,	//	0x200:
    ctrl_fill_step1,	//	0x201:
    ctrl_fill_step2,	//	0x202:
    ctrl_fill_step3,	//	0x203:
    ctrl_fill_step4,	//	0x204:
    ctrl_fill_step5,	//	0x205:

    ctrl_empty = 0x300, //	0x300:
    ctrl_empty_step1,   //	0x301:
    ctrl_empty_step2,   //	0x302:
    ctrl_empty_step3,   //	0x303:
    ctrl_empty_step4,   //	0x304:
    ctrl_empty_step5,   //	0x305:
	ctrl_empty_step6,   //	0x306:
	
	ctrl_cal_start = 0x400, // 0x400: 
	ctrl_cal_step1,    		// 0x401: Evaluate PID-Offset to pump 40ml/min
    ctrl_cal_step2,  		// 0x402:
    ctrl_cal_step3,   		// 0x403:
    ctrl_cal_step4,   		// 0x404:
	ctrl_cal_done,       	// 0x405:

	ctrl_recovery_start = 0x500,	// 0x500
	ctrl_recovery_step1,			// 0x501
	ctrl_recovery_step2,			// 0x502
	ctrl_recovery_step3,			// 0x503
	ctrl_recovery_step4,			// 0x504
	ctrl_recovery_step5,			// 0x505
	ctrl_recovery_step6,			// 0x506
	ctrl_recovery_step7,			// 0x507
    ctrl_recovery_step8,			// 0x508
    ctrl_recovery_step9,			// 0x509

    ctrl_test_watchdog = 0x10000,
    ctrl_test, 					// 0x10001
    ctrl_test_valve, 			// 0x10002
    ctrl_test_heater, 			// 0x10003

    ctrl_toggle_meniscus = 0x20000,
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

typedef struct SPurgePar
{
	INT32 no;
    // INT32 delay;
    INT32 delay_pos_y;
    INT32 act_pos_y;
	INT32 time;
    INT32 delay_time;
} SPurgePar;

int valid(int val);

char *value_str(int val);
char *value_str1(int val);
char *value_str3(int val);
char *value_str_temp(int val);
char *value_str_time(int sec);
char *value_str_u(int val);
char *value_str_screw(INT16 val);
int str_to_screw(const char *str);

//INT32 convert_i16_to_i32(INT16 val);

#endif // __RX_INK_COMMON__
