// Bluetooth defines 
// Original file location: ... RX-Project\rx_digiprint_gui\Services\RxBtDef.cs

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Services
{
    public class RxBtDef
    {
        public static readonly string MouventBlueToothGuid = "BE484CFE-9945-4293-BD91-E1D930C3FBF2";
        public static readonly string InfoPwd              = "Mouvent";
        public static readonly string LicPwd               = "R@dex";
        public static readonly string LicFileName          = "mouvent.lic";

        public const UInt32 INVALID_VALUE       = 0x5555aaaa;

        //--- COMMAND IDs ------------------------------------------
        public const UInt32 BT_CMD_LOGIN		= 0x01010001;
        public const UInt32 BT_REP_LOGIN		= 0x02010001;

        public const UInt32 BT_CMD_STATE	    = 0x01010002;
        public const UInt32 BT_REP_STATE	    = 0x02010002;

        public const UInt32 BT_CMD_HEAD_STATE	= 0x01010003;
        public const UInt32 BT_REP_HEAD_STATE	= 0x02010003;

        public const UInt32 BT_EVT_GET_EVT      = 0x03010111;
        public const UInt32 BT_CMD_EVT_CONFIRM	= 0x01010112;
        public const UInt32 BT_REP_EVT_CONFIRM  = 0x02010112;

        public const UInt32 BT_CMD_PROD_STATE   = 0x01010211;
        public const UInt32 BT_EVT_PROD_STATE   = 0x03010211;
        public const UInt32 BT_CMD_PROD_PREVIEW = 0x01010212;
        public const UInt32 BT_REP_PROD_PREVIEW = 0x02010212;

        public const UInt32 CMD_START_PRINTING	= 0x01000301;
        public const UInt32 REP_START_PRINTING	= 0x02000301;
        
        public const UInt32 CMD_PAUSE_PRINTING	= 0x01000302;
        public const UInt32 REP_PAUSE_PRINTING	= 0x02000302;

        public const UInt32 CMD_STOP_PRINTING	= 0x01000303;
        public const UInt32 REP_STOP_PRINTING	= 0x02000303;

        public const UInt32 CMD_ABORT_PRINTING	= 0x01000304;
        public const UInt32 REP_ABORT_PRINTING	= 0x02000304;

        public const UInt32 CMD_FLUID_CTRL_MODE = 0x01000401;

        public const UInt32 BT_CMD_STEPPER_STATE= 0x01010501;
        public const UInt32 BT_REP_STEPPER_STATE= 0x02010501;
        public const UInt32 BT_CMD_STEPPER_TEST = 0x01010502;
        public const UInt32 BT_CMD_STEPPER_STOP = 0x01010503;
        public const UInt32 BT_CMD_STEPPER_REF  = 0x01010504;
        public const UInt32 BT_CMD_STEPPER_UP   = 0x01010505;
        public const UInt32 BT_CMD_STEPPER_PRINT= 0x01010506;
        public const UInt32 BT_CMD_STEPPER_CAP  = 0x01010507;

        //--- WEB-IN ---------------------
        public const UInt32 BT_CMD_JOG_FWD_START= 0x01010610;
        public const UInt32 BT_CMD_JOG_FWD_STOP = 0x01010611;
        public const UInt32 BT_CMD_JOG_BWD_START= 0x01010612;
        public const UInt32 BT_CMD_JOG_BWD_STOP = 0x01010613;

        public const UInt32 BT_CMD_BUTTON_1		= 0x01010710;
        public const UInt32 BT_CMD_BUTTON_2		= 0x01010711;
        public const UInt32 BT_CMD_BUTTON_3		= 0x01010712;
        public const UInt32 BT_CMD_BUTTON_4		= 0x01010713;


        //--- SMsgHdr ----------------------------------
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SMsgHdr
        {
            public Int32  msgLen;
            public UInt32 msgId;
        }

        //--- EBTState ----------------------------
        public enum EBTState : int
        {
            state_undef,	    // 00
            state_offline,      // 01
	        state_webin,	    // 02
            state_stop,         // 03
            state_prouction,    // 04
        }

        //--- EUserType -----------------------------
        public enum EUserType : byte
        {
       	    usr_undef,	    // 00
	        usr_operator,	// 01
	        usr_supervisor,	// 02
	        usr_service,	// 03	
            usr_mouvent,    // 04
            usr_end         // 05
        };

        //--- ELogType ----------------------------
        public enum ELogType : byte
        {
            eErrUndef   = 0,
            eErrLog     = 1,
            eErrWarn    = 2,
            eErrCont    = 3,
            eErrStop    = 4,
            eErrAbort   = 5,
        };

        //--- SReplyStateMsg ------------------------------------
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SReplyStateMsg
        {
            public SMsgHdr hdr;
            public EBTState state;
            public Int32    colorCnt;
            public Int32    headsPerColor;
            public Int32    reverse;
        };

        //--- SReplyMsg ------------------------------------
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SReplyMsg
        {
            public SMsgHdr hdr;
            public Int32 reply;
        };

        //--- SErrorMsg --------------------------------------
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SErrorMsg
        {
            public SMsgHdr  hdr;
            public ELogType type;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string   device;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string   message;
        }

        //--- SLogInMsg ------------------------------------
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SLogInMsg
        {
            public SMsgHdr hdr;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string connectionId;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
            public string userName;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string license;
        };

        //--- EPQState --------------------------
        public enum EPQState : byte
        {
		    undef,     //	0
		    queued,    //	1
		    preflight, // 	2
		    ripping,   // 	3
		    screening, // 	4
            loading,   //   5
		    transfer,  // 	6
		    buffered,  // 	7
		    printing,  //	8
		    printed,   //   9	
            stopping,  //  10
            stopped    //  11
        };

        //--- EButtonState --------------------------
        public enum EButtonState : byte
        {
            isEnabled = 0x01,
            isChecked = 0x02,
            isLarge   = 0x04,
        };

        //--- SProdStateMsg ------------------------------------
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SProdStateMsg
        {
            public SMsgHdr hdr;
            
            public EButtonState startBnState;
            public EButtonState pauseBnState;
            public EButtonState stopBnState;
            public EButtonState abortBnState;
            public bool         allInkSuppliesOn;    

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string filepath;

            public EPQState state;
            public Int32 progress;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string progressStr;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string copiesStr;
        };

        public enum EFluidCtrlMode : int
        {
	        ctrl_undef,				//  0x000:
            ctrl_shutdown,          //  0x001:
            ctrl_shutdown_done,     //  0x002:
            ctrl_error,             //  0x003:
            ctrl_wait,              //  0x004:
	        ctrl_off,				//	0x005:
	        ctrl_warmup,			//  0x006:
	        ctrl_readyToPrint,		//  0x007:
            ctrl_prepareToPrint,    //  0x008:
	        ctrl_print,				//  0x009:
	        ctrl_bleed,				//  0x00a:
            ctrl_0b, 		        //  0x00b:
            ctrl_flush_0,	        //	0x010:
            ctrl_flush_1,	        //	0x011:
            ctrl_flush_2,	        //	0x012:
	        ctrl_flush_step1,	    //  0x013:
	        ctrl_flush_step2,	    //  0x014:
            ctrl_flush_done,        //	0x015:

	        ctrl_purge_soft =0x100,	//	0x100:
	        ctrl_purge,				//	0x101:
	        ctrl_purge_hard,		//  0x102:
	        ctrl_purge_micro,		//  0x103:
	        ctrl_purge_step1=0x111,	//	0x111:
	        ctrl_purge_step2,		//	0x112:
	        ctrl_purge_step3,		//	0x113:

            ctrl_wipe       =0x120, //  0x120   

	        ctrl_fill	    =0x200,	//	0x200:
	        ctrl_fill_step1,		//	0x201:
	        ctrl_fill_step2,		//	0x202:
	        ctrl_fill_step3,		//	0x203:
	        ctrl_fill_step4,		//	0x204:
	        ctrl_fill_step5,		//	0x205:

	        ctrl_empty		=0x300, //	0x300:
	        ctrl_empty_step1,		//	0x301:
	        ctrl_empty_step2,		//	0x302:
	        ctrl_empty_step3,		//	0x303:
	        ctrl_empty_step4,		//	0x304:
	        ctrl_empty_step5,		//	0x305:
            ctrl_empty_step6,       //	0x306:

            ctrl_cal_start = 0x400, // 0x400:
            ctrl_cal_step1,    	    // 0x401:
            ctrl_cal_step2,  	    // 0x402:
            ctrl_cal_step3,   	    // 0x403:
            ctrl_cal_step4,   	    // 0x404:
	        ctrl_cal_done,       	// 0x405:

	        ctrl_test_watchdog = 0x10000,
	        ctrl_test,				// 0x10001
            ctrl_reset_pumptime, 	// 0x10002
            ctrl_offset_cal,	    // 0x10003
            ctrl_offset_cal_done,	// 0x10004
            ctrl_factory_offset_cal,// 0x10005
            ctrl_del_user_offset,   // 0x10006
        };

        //--- SHeadStat ---------------------------------
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SBtHeadStat
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 8)]
            public string           name;
            public UInt32           color;
            public UInt32           colorFG;

	        //--- ink system ---------------------------------
	        public Int32			temp;
            public Int32			meniscus;
	        public EFluidCtrlMode   ctrlMode;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SBtFluidCtrlCmd
        {
	        public SMsgHdr          hdr;
	        public Int32			no;
	        public EFluidCtrlMode   ctrlMode;
        };

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SBtHeadStatMsg
        {
            public SMsgHdr          hdr;
            public Int32            no;
            public SBtHeadStat      status;
        }

        //--- SStepperStat ---------------------------------
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SBtStepperStat
        {
            public Double motorPos;
            public Double encoderPos;
            public UInt32 encoderColor;
            public UInt32 endSwitch;
            public UInt32 state;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SBtStepperStatMsg
        {
            public SMsgHdr          hdr;
            public Int32            no;
            public SBtStepperStat   status;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SBtStepperTestMsg
        {
            public SMsgHdr          hdr;
            public Int32            motorNo;
            public Int32            moveUp;
        }

        //--- StrCrc -----------------------------
        public static char StrCrc(string str)
        {
            int crc=str.Length;
            foreach (char ch in str)
            {
                crc=3*crc+ch;
            }
            return (char)(0x20+(crc%0x60));
        }        

    }
}
