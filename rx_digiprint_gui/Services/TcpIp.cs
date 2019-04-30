using System;
using System.Runtime.InteropServices;

namespace RX_DigiPrint.Services
{
    public enum ELogType : byte
    {
        eErrUndef   = 0,
        eErrLog     = 1,
        eErrWarn    = 2,
        eErrCont    = 3,
        eErrStop    = 4,
        eErrAbort   = 5,
    };

    public enum ERectoVerso : int
    {
	    rv_undef    = 0,	// 00
	    rv_recto    = 1,	// 01
	    rv_verso    = 2,	// 02
    };

    public enum EDeviceType : byte
    {
       	dev_undef,	// 00
	    dev_main,	// 02
	    dev_gui,	// 01
	    dev_plc,	// 08
	    dev_enc,	// 03	
	    dev_fluid,	// 05
        dev_6,      // 06
        dev_7,      // 07
        dev_8,      // 08
        dev_stepper,// 09
	    dev_head,	// 10
	    dev_spool,	// 11
        dev_end     // 12
    };

    public enum EUserType : byte
    {
       	usr_undef,	    // 00
	    usr_operator,	// 01
	    usr_supervisor,	// 02
	    usr_service,	// 03	
        usr_mouvent,    // 04
        usr_end         // 05
    };

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

    public enum EPQLengthUnit : byte
    {
        undef,  //  0
        mm,     //  1
        copies  //  2
    };

    public enum EScanMode : byte
    {
        scan_std,   //  0
        scan_rtl,   //  1
        scan_bidir  //  2
    };

    public enum EPrintState : int
    {
	    ps_undef,		// 00
	    ps_off,			// 01
	    ps_ready_power, // 02
	    ps_printing,	// 03
	    ps_stopping,	// 04
        ps_goto_pause,  // 05
        ps_pause,       // 06
        ps_cleaning,    // 07
        ps_webin,       // 08
        ps_setup,       // 09
    };

    public enum EPrinterType : int
    {
	    printer_undef,				// 0: not defined
	    printer_test_table,			// 1:
        printer_test_slide,         // 2:
        printer_test_slide_only,    // 3: 

        //--- web printers ------------------------------
        printer_LB701    =1000,	    // 1000: 
        printer_LB702_UV,	        // 1001: 
        printer_LB702_WB,	        // 1002: 

        printer_DP803    =1100,     // 1100:
	 
	    //--- scanning printers --------------------------
	    printer_TX801     = 2000,		// 2000: Fashionn stanrard output
	    printer_TX802,				// 2001: Fashion high outpput

	    //--- special projects ----------------
	    printer_cleaf = 10000,		// 10000: Cleaf machine	
    };

    public enum ETestImage : byte
    {
        undef,                  // 00
        angle_overlap,          // 01
        angle_separated,        // 02
        jets,                   // 03
        jet_numbers,            // 04
        grid,                   // 05
        encoder,                // 06
        scanning,               // 07
    };

    public enum EFluidCtrlMode : int
    {
	    ctrl_undef,				//  0x000:
        ctrl_shutdown,          //  0x001:
  //      ctrl_shutdown_done,     //  0x002:
        ctrl_error          = 0x002,
	    ctrl_off            = 0x003,	//	0x003:
	    ctrl_warmup,			//  0x004:
	    ctrl_readyToPrint,		//  0x005:
	    ctrl_print,				//  0x006:
	    ctrl_bleed,				//  0x007:
        ctrl_08,                //  0x008:
        ctrl_09,        	    //  0x009:
        ctrl_0a,      		    //	0x00a:
        ctrl_0b, 		        //  0x00b:

        ctrl_flush_night,	    //	0x010:
        ctrl_flush_weekend,	    //	0x011:
        ctrl_flush_week,	    //	0x012:
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

	    ctrl_cal_start = 0x400, // 0x400:
	    ctrl_cal_step1,    	    // 0x401:
        ctrl_cal_step2,  	    // 0x402:
        ctrl_cal_step3,   	    // 0x403:
        ctrl_cal_step4,   	    // 0x404:
	    ctrl_cal_done,       	// 0x405:

	    ctrl_test_watchdog = 0x10000,
	    ctrl_test,				// 0x10001
        ctrl_offset_cal, 	    // 0x10002
        ctrl_offset_cal_done,	// 0x10003
        ctrl_offset_del_factory,// 0x10004
        ctrl_offset_del_user,   // 0x10005
    };

    public enum ETestTableErr : uint
    {
        fpga_not_loaded = 0x00000001,
		nios_not_loaded	= 0x00000002,
		stepper_left	= 0x00000004,
		stepper_right	= 0x00000008,
		slide			= 0x00000010        
    }

    public class TcpIp
    {        
        public static string RX_CTRL_MAIN = "192.168.200.1";
//        public static string RX_DATA_SOURCE_ROOT = "\\\\"+RX_CTRL_MAIN+"\\Source Data";    
        public static string RX_SOURCE_DATA_ROOT = "\\source-data";    
        public static string RX_RIPPED_DATA_ROOT = "\\ripped-data";    

        public const Int32 InkSupplyCnt = 16;
        public const UInt32 HEAD_CNT	 =	4;

        public const Int32 PORT_GUI = 7000;

        public const UInt32 CMD_X = 0x01000000; // commands
        public const UInt32 REP_X = 0x02000000;	// replies to commands
        public const UInt32 EVT_X = 0x03000000;	// events

        public const UInt32 INVALID_VALUE       = 0x5555aaaa;
        public const UInt32 VAL_UNDERFLOW       = 0x5555aaab;
        public const UInt32 VAL_OVERFLOW        = 0x5555aaac;

        public const UInt32 CMD_PING            = 0x01000001;
        public const UInt32 REP_PING            = 0x02000001;

        public const UInt32 CMD_STATUS          = 0x01000002;
        public const UInt32 REP_STATUS          = 0x02000002;
        public const UInt32 EVT_STATUS          = 0x03000002;

        public const UInt32 CMD_RESTART_MAIN    = 0x01000004;

        public const UInt32 CMD_GET_EVT         = 0x01000011;
        public const UInt32 REP_GET_EVT         = 0x02000011;
        public const UInt32 EVT_GET_EVT         = 0x03000011;

        public const UInt32 CMD_EVT_CONFIRM		= 0x01000012;
        public const UInt32 REP_EVT_CONFIRM		= 0x02000012;

        public const UInt32 CMD_REQ_LOG			= 0x01000013;
        public const UInt32 REP_REQ_LOG			= 0x02000013;
        public const UInt32 EVT_GET_LOG			= 0x03000013;

        public const UInt32 CMD_GET_NETWORK	    = 0x01000031;	// reqeuests all network items
        public const UInt32 REP_GET_NETWORK		= 0x02000031;	// reply
        public const UInt32 EVT_NETWORK_ITEM	= 0x03000031;	// send one network item
        
        public const UInt32 CMD_SET_NETWORK		= 0x01000032;	// set/change one network item
        public const UInt32 REP_SET_NETWORK     = 0x02000032;	// reply
        public const UInt32 CMD_NETWORK_SAVE    = 0x01000033;
        public const UInt32 CMD_NETWORK_RELOAD  = 0x01000034;
        public const UInt32 REP_NETWORK_RELOAD  = 0x02000034; 
        public const UInt32 CMD_NETWORK_DELETE  = 0x01000035;

        public const UInt32 CMD_NETWORK_SETTINGS= 0x01000036;	// set / Change settings
        public const UInt32 REP_NETWORK_SETTINGS= 0x02000036;	// receive settings

        public const UInt32 CMD_GET_PRINT_QUEUE	    = 0x01000041;
        public const UInt32 REP_GET_PRINT_QUEUE	    = 0x02000041;
        public const UInt32 EVT_GET_PRINT_QUEUE     = 0x03000041;
        public const UInt32 CMD_GET_PRINT_QUEUE_ITM = 0x04000041;

        public const UInt32 CMD_ADD_PRINT_QUEUE = 0x01000042;
        public const UInt32 REP_ADD_PRINT_QUEUE = 0x02000042;

        public const UInt32 CMD_SET_PRINT_QUEUE = 0x01000043;
        public const UInt32 REP_SET_PRINT_QUEUE = 0x02000043;

        public const UInt32 CMD_DEL_PRINT_QUEUE = 0x01000044;
        public const UInt32 REP_DEL_PRINT_QUEUE = 0x02000044;
        public const UInt32 EVT_DEL_PRINT_QUEUE = 0x03000044;

        public const UInt32 CMD_UP_PRINT_QUEUE  = 0x01000045;
        public const UInt32 REP_UP_PRINT_QUEUE  = 0x02000045;
        public const UInt32 EVT_UP_PRINT_QUEUE  = 0x03000045;

        public const UInt32 CMD_DN_PRINT_QUEUE  = 0x01000046;
        public const UInt32 REP_DN_PRINT_QUEUE  = 0x02000046;
        public const UInt32 EVT_DN_PRINT_QUEUE  = 0x03000046;

        public const UInt32 CMD_GET_PRINT_ENV   = 0x01000047;
        public const UInt32 REP_GET_PRINT_ENV   = 0x02000047;
        public const UInt32 BEG_GET_PRINT_ENV   = 0x04000047;
        public const UInt32 ITM_GET_PRINT_ENV   = 0x05000047;
        public const UInt32 END_GET_PRINT_ENV   = 0x06000047;

        public const UInt32 CMD_GET_INK_DEF     = 0x01000048;
        public const UInt32 REP_GET_INK_DEF     = 0x02000048;
        public const UInt32 BEG_GET_INK_DEF     = 0x04000048;
        public const UInt32 ITM_GET_INK_DEF     = 0x05000048;
        public const UInt32 END_GET_INK_DEF     = 0x06000048;

        public const UInt32 CMD_GET_PRINTER_CFG = 0x01000049;
        public const UInt32 REP_GET_PRINTER_CFG = 0x02000049;
        public const UInt32 CMD_SET_PRINTER_CFG = 0x0100004A;
        public const UInt32 REP_SET_PRINTER_CFG = 0x0200004A;

        public const UInt32 CMD_DEL_FILE        = 0x0100004B;

        public const UInt32  CMD_HEAD_STAT			= 0x01000103;
        public const UInt32  REP_HEAD_STAT			= 0x02000103;
        
        public const UInt32  CMD_HEAD_FLUID_CTRL_MODE = 0x01000104;
        public const UInt32  REP_HEAD_FLUID_CTRL_MODE = 0x02000104;

        public const UInt32 CMD_ENCODER_CFG			= 0x01000111;
        public const UInt32 REP_ENCODER_CFG			= 0x02000111;

        public const UInt32 CMD_ENCODER_STAT		= 0x01000112;
        public const UInt32 REP_ENCODER_STAT		= 0x02000112;

        public const UInt32 CMD_ENCODER_SAVE_PAR	= 0x01000119;
        public const UInt32 CMD_ENCODER_SAVE_PAR_1	= 0x0100011A;

        public const UInt32  CMD_ENCODER_UV_ON		  = 0x01000113;
        public const UInt32  CMD_ENCODER_UV_OFF		  = 0x01000114;

        public const UInt32  CMD_FLUID_STAT			= 0x01000122;
        public const UInt32  REP_FLUID_STAT			= 0x02000122;

        public const UInt32  CMD_FLUID_CTRL_MODE    = 0x01000124;
        public const UInt32  REP_FLUID_CTRL_MODE	= 0x02000124;

        public const UInt32  CMD_FLUID_PRESSURE     = 0x01000125;

        public const UInt32 CMD_GET_STEPPER_CFG		= 0x01000131;
        public const UInt32 REP_GET_STEPPER_CFG		= 0x02000131;
        public const UInt32 CMD_SET_STEPPER_CFG		= 0x01000132;
        public const UInt32 REP_SET_STEPPER_CFG		= 0x02000132;

        public const UInt32 CMD_SCALES_SET_CFG		= 0x01000141;
        public const UInt32 CMD_SCALES_GET_CFG		= 0x01000142;
        public const UInt32 REP_SCALES_GET_CFG		= 0x02000142;
        public const UInt32 CMD_SCALES_TARA		    = 0x01000143;
        public const UInt32 CMD_SCALES_STAT		    = 0x01000144;
        public const UInt32 REP_SCALES_STAT	        = 0x02000144;

        public const UInt32 CMD_BCSCANNER_RESET	    = 0x01000145;
        public const UInt32 CMD_BCSCANNER_IDENTIFY	= 0x01000146;
        public const UInt32 CMD_BCSCANNER_TRIGGER	= 0x01000147;

        public const UInt32 REP_CHILLER_STAT		= 0x02000152;

        public const UInt32 CMD_START_PRINTING		= 0x01000201;
        public const UInt32 REP_START_PRINTING		= 0x02000201;
        
        public const UInt32 CMD_PAUSE_PRINTING		= 0x01000202;
        public const UInt32 REP_PAUSE_PRINTING		= 0x02000202;

        public const UInt32 CMD_STOP_PRINTING		= 0x01000203;
        public const UInt32 REP_STOP_PRINTING		= 0x02000203;

        public const UInt32 CMD_ABORT_PRINTING		= 0x01000204;
        public const UInt32 REP_ABORT_PRINTING		= 0x02000204;

        public const UInt32 CMD_EXTERNAL_DATA_ON    = 0x01000205;
        public const UInt32 CMD_EXTERNAL_DATA_OFF   = 0x01000206;

        public const UInt32 CMD_RESET_CTR			= 0x01000207;

        public const UInt32 CMD_CLEAN_START 	    = 0x01000210;
        public const UInt32 REP_CLEAN_SATRT	        = 0x02000210;

        public const UInt32 CMD_TEST_START 	        = 0x01000211;
        public const UInt32 REP_TEST_SATRT	        = 0x02000211;

        public const UInt32 CMD_GET_PRN_STAT	    = 0x01000220;
        public const UInt32 EVT_PRINTER_STAT	    = 0x03000220;

        //--- plc cpommands ------------------------------------------
        public const UInt32 CMD_PLC_RSEET_ERROR     = 0x01000301;
        public const UInt32 CMD_PLC_GET_INFO		= 0x01000350;
        public const UInt32 REP_PLC_GET_INFO		= 0x02000350;
        public const UInt32 CMD_PLC_RESET_ERROR		= 0x01000351;
        public const UInt32 CMD_PLC_REBOOT			= 0x01000352;

        public const UInt32 CMD_PLC_GET_LOG			= 0x01000353;
        public const UInt32 REP_PLC_GET_LOG			= 0x02000353;
 //       public const UInt32 END_PLC_GET_LOG			= 0x06000353;

        public const UInt32 CMD_PLC_SAVE_PAR        = 0x01000360;
        public const UInt32 CMD_PLC_LOAD_PAR        = 0x01000361;
        public const UInt32 REP_PLC_LOAD_PAR        = 0x02000361;
        public const UInt32 CMD_PLC_GET_VAR         = 0x01000362;
        public const UInt32 REP_PLC_GET_VAR         = 0x02000362;
        public const UInt32 CMD_PLC_SET_VAR         = 0x01000363;
        public const UInt32 CMD_PLC_SET_CMD         = 0x01000364;
        public const UInt32 CMD_PLC_REQ_MATERIAL    = 0x01000365;
        public const UInt32 CMD_PLC_RES_MATERIAL    = 0x02000365;
        public const UInt32 CMD_PLC_ITM_MATERIAL    = 0x03000365;
        public const UInt32 CMD_PLC_SAVE_MATERIAL   = 0x01000366;
        public const UInt32 CMD_PLC_DEL_MATERIAL    = 0x01000367;

        //--- STEPPER -------------------------------------------------
        public const UInt32 CMD_STEPPER_TEST		= 0x01000403;

        //--- Test Table Commands ------------------------------------------
        public const UInt32 CMD_TT_STOP				= 0x01000501;
        public const UInt32 CMD_TT_START_REF		= 0x01000502;
        public const UInt32 CMD_TT_MOVE_TABLE		= 0x01000503;
        public const UInt32 CMD_TT_SCAN_RIGHT		= 0x01000504;
        public const UInt32 CMD_TT_SCAN_LEFT		= 0x01000505;
        public const UInt32 CMD_TT_VACUUM			= 0x01000506;
        public const UInt32 CMD_TT_SCAN			    = 0x01000507;
        public const UInt32 CMD_TT_MOVE_LOAD	    = 0x01000508;
        public const UInt32 CMD_TT_MOVE_CAP	        = 0x01000509;
        public const UInt32 CMD_TT_MOVE_PURGE	    = 0x0100050a;
        public const UInt32 CMD_TT_MOVE_ADJUST	    = 0x0100050b;
        public const UInt32 CMD_TT_STAT		        = 0x01000510;
        public const UInt32 REP_TT_STAT		        = 0x02000510;

        //--- Capping Commands ---------------------------------------------------
        public const UInt32 CMD_CAP_STOP			= 0x01000601;
        public const UInt32 CMD_CAP_REFERENCE		= 0x01000602;
        public const UInt32 CMD_CAP_PRINT_POS		= 0x01000603;
        public const UInt32 CMD_CAP_CAPPING_POS		= 0x01000604;
        public const UInt32 CMD_CAP_UP_POS  		= 0x01000605;

        public const UInt32 CMD_CLN_STOP			= 0x01000701;
        public const UInt32 CMD_CLN_REFERENCE		= 0x01000702;
        public const UInt32 CMD_CLN_MOVE_POS		= 0x01000703;
        public const UInt32 CMD_CLN_SCREW_REF		= 0x01000704;
        public const UInt32 CMD_CLN_SCREW_0_POS		= 0x01000705;
        public const UInt32 CMD_CLN_SCREW_1_POS		= 0x01000706;
        public const UInt32 CMD_CLN_SCREW_2_POS		= 0x01000707;
        public const UInt32 CMD_CLN_SCREW_3_POS		= 0x01000708;
        public const UInt32 CMD_CLN_WIPE			= 0x01000709;

        public const UInt32 CMD_CLN_ADJUST			= 0x01000710;

        public const UInt32 CMD_CLN_DRIP_PANS       = 0x01000721;

        public const UInt32 EVT_TRACE               = 0x03000100;

        //--- cleaf orders -----------------------------------------
        public const UInt32 CMD_CO_SET_ORDER		= 0x01003000;
        public const UInt32 CMD_CO_GET_ORDER		= 0x01003001;
        public const UInt32 REP_CO_GET_ORDER		= 0x02003001;
        public const UInt32 CMD_CO_SET_OPERATOR		= 0x01003002;
        public const UInt32 REP_CO_SET_OPERATOR		= 0x02003002;
        public const UInt32 CMD_CO_GET_PRODUCTION	= 0x02003003;
        public const UInt32 REP_CO_GET_PRODUCTION	= 0x02003003;
        public const UInt32 CMD_CO_GET_ROLLS		= 0x01003004;
        public const UInt32 REP_CO_GET_ROLLS		= 0x02003004;
        public const UInt32 CMD_CO_SET_ROLL			= 0x01003005;
        public const UInt32 REP_CO_SET_ROLL			= 0x02003005;

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SVersion
        {
	        public UInt32 major;		// 
	        public UInt32 minor;		// 
	        public UInt32 revision;	    // SVN-Revision number 
	        public UInt32 build;		// incremented by each compilation
        };

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SMsgHdr
        {
            public Int32  msgLen;
            public UInt32 msgId;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SValue
        {
            public SMsgHdr hdr;
	        public Int32	no;
	        public Int32	value;
        };

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SErrorMsg
        {
            public SMsgHdr hdr;

            public EDeviceType      devType;
            public byte             devNo;
            public ELogType         logType;
            public byte             res1;
            public Int64            time;
            public UInt32           errNo;
            public UInt32           line;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string file;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string formatStr;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 256)]
            public byte[] args;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SLogReqMsg
        {
            public SMsgHdr hdr;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string filepath;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string find;
            public Int32 first;  
            public Int32 count;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SNetworkItem
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string           deviceTypeStr;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string           serialNo;
            public UInt64           macAddr;
            public EDeviceType      devType;
            public byte             devNo;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string           ipAddr;
            public byte             connected;
            public byte             platform;
            public UInt32           rfsPort;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
            public UInt32[]	        ports;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct sNetworkMsg
        {
            public SMsgHdr      hdr;
            public SNetworkItem item; 
            public byte         flash;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct sSetNetworkCmd
        {
            public SMsgHdr      hdr;
            public SNetworkItem item;
            public byte         flash;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SIfConfig
        {
            public SMsgHdr      hdr;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
	        public string	hostname;
	        public UInt32	dhcp;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
            public byte[]   addr;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
            public byte[]   mask;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SPageId
        {
        	public Int32 id;
	        public Int32 page;
	        public Int32 copy;
	        public Int32 scan;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct sPageNumber
        {
            public UInt32 enabled;
            public UInt32 number;
            public UInt32 fontSize;
            public UInt32 imgDist;
        }

        public enum EPrintGoMode : int
        {
            PG_MODE_MARK	=1,	// print mark to PG
			PG_MODE_LENGTH,		// distance between two GPs
			PG_MODE_GAP,        // gap getween images        
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct sPrintQueueItem
        {
	        public SPageId	id;
	        public SPageId	start;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
	        public string	filepath;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
	        public string	preview;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string   ripState;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
            public string   printEnv;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
            public string   material;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
            public string   testMessage;

	        public Int32	srcPages;
	        public Int32	srcWidth;
	        public Int32	srcHeight;

	        public Int32	firstPage;
	        public Int32	lastPage;
	        public Int32    copies;
	        public byte	    collate;
	        public byte	    variable;
	        public byte	    dropSizes;

	        public EPQState	     state;
            public EPQLengthUnit lengthUnit;
            public EScanMode    scanMode;
	        public byte	        orientation;
            public byte         testImage;
	        public Int32	    pageWidth;
	        public Int32	    pageHeight;
	        public Int32	    pageMargin;
	        public EPrintGoMode printGoMode;
	        public Int32        printGoDist;
	        public Int32	    scanLength;
	        public byte	        passes;
            public byte         virtualDoublePass;
	        public byte	        curingPasses;
	        public Int32	    scans;
            public Int32        speed;

            public Int32   copiesTotal;
            public Int32   copiesPrinted;
            public Int32   scansSent;
            public Int32   scansPrinted;
            public Int32   scansStart;
            public Int32   scansTotal;
            public Int32   progress;

            public sPageNumber pageNumber;

            public Int32   checks;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 4)]
            public string   dots;
	        public byte	    wakeup;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct SWfPoint
        {
	        private UInt16	pos;
	        private UInt16 volt;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SInkDefinition
        { 
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst =64)]
            public string  family;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst =64)]
            public string  fileName;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst =64)]
            public string  name;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst =128)]
            public string  description;
            public byte	b;
            public byte	g;
            public byte	r;
            public byte	a;
            public UInt32 colorCode;

            Int32	temp;
	        Int32	tempMax;
	        Int32	dropletVolume;
	        Int32	meniscus;
            Int32 condPresOut;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	        public Int32[]	flushTime;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	        Int32[]	maxFreq;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	        byte[]	greyLevel;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
	        SWfPoint[] wf;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct sPrintQueueMsg
        {
            public SMsgHdr hdr;
            public sPrintQueueItem item;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct sPrintEnvMsg
        {
            public SMsgHdr hdr;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst =128)]
            public string  printEnv;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SInkDefMsg
        {
            public SMsgHdr          hdr;
            public SInkDefinition   ink;
            UInt32			        headNo;
	        UInt32			        maxDropSize;
	        UInt32			        fpVoltage;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SOffsetCfg
        {
	        public Int32	angle;	// to adjust angle fault
	        public Int32	step;	    // to adjust steph fault
	        public Int32	incPerMeter;
	        public Int32	incPerMeterVerso;
            public Int32    versoDist;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SPrinterCfgMsg
        {
            public SMsgHdr      hdr;
            public EPrinterType	type;
            public UInt32		overlap;
            public SOffsetCfg   offset;
            public Int32        externalData;		
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst =16*64)]
	        public string       inkFileNames;     // comma separated list
            [MarshalAs(UnmanagedType.ByValArray, SizeConst =16)]	
            public ERectoVerso[]    rectoVerso;
            public Int32		headsPerColor;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst =128)]	
            public Int32[]		headFpVoltage;	
            [MarshalAs(UnmanagedType.ByValArray, SizeConst =128)]	
            public Int32[]		headDist;	
            [MarshalAs(UnmanagedType.ByValArray, SizeConst =128)]	
            public Int32[]		headDistBack;	
            [MarshalAs(UnmanagedType.ByValArray, SizeConst =16)]	
            public Int32[]		colorOffset;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SPrinterStatus
        {
            public EPrintState  printState;
            public byte         error;
            public UInt32       dataReady;
            public UInt32       sentCnt;
            public UInt32       transferredCnt;
            public UInt32       printedCnt;
            public UInt32       testMode;
            public UInt32		inkSupilesOff;
	        public UInt32		inkSupilesOn;	
            [MarshalAs(UnmanagedType.ByValArray, SizeConst =4)]
            public UInt32[]     maxSpeed;   // [m/min]
	        public UInt32		externalData;
            public UInt32		actSpeed;
            public double       counterAct;
            public double       counterTotal;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SRobotOffsets
        {
            public Int32       ref_height;
            public Int32       head_align;
            public Int32       robot_height;
            public Int32       robot_align;
        }
        
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SStepperCfgMsg
        {
            public SMsgHdr      hdr;
        	public EPrinterType	printerType;    // used in background only
	        public Int32		boardNo;        // used in background only
	        public Int32		ref_height;
	        public Int32		print_height;
	        public Int32		wipe_height;
	        public Int32		cap_height;
	        public Int32		cap_pos;
	        public Int32		adjust_pos;
	        public Int32		use_printhead_en;

            [MarshalAs(UnmanagedType.ByValArray, SizeConst =4)]
            public SRobotOffsets[] robot;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SStepperMotorTestMsg
        {
            public SMsgHdr      hdr;
            public Int32        boardNo;
            public Int32        motorNo;
            public Int32        microns;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SHeadAdjustmentMsg
        {
            public SMsgHdr      hdr;
            public Int32        inkSupplyNo;
            public Int32        headNo;
            public Int32        angle;
            public Int32        stitch;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SPrinterStatusMsg
        {
            public SMsgHdr hdr;
            public SPrinterStatus status;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SInkSupplyStat
        {
            public UInt32	info;
            public UInt32	warn;
	        public UInt32	err;

	        public Int32	cylinderPresSet;	//  Pressure intermediate Tank
	        public Int32	cylinderPres;	    //  Pressure intermediate Tank
            public Int32    cylinderSetpoint;	 //  Pressure intermediate Tank
            public Int32    airPressureTime;
            public Int32	flushTime;
	        public Int32    presLung;			//  Lung pressure
            public Int32    condPresOut;  
            public Int32    condPresIn;
	        public UInt32	temp;				//	Temperature
	        public UInt32	pumpSpeedSet;		//	Consumption pump speed
	        public UInt32	pumpSpeed;			//	Consumption pump speed
	        public Int32	canisterLevel;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
            public string   scannerSN;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string   barcode;
 	        public EFluidCtrlMode	ctrlMode;	//	EnFluidCtrlMode
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SInkSupplyStatMsg
        {
            public SMsgHdr hdr;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 18)]
            public SInkSupplyStat[] status;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SChillerStat
        {
        	public Int32	enabled;
        	public Int32	running;
        	public Int32	tempSet;
	        public Int32	temp;
	        public Int32	pressure;
	        public Int32	resistivity;
            public UInt32	status;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SChillerStatMsg
        {
            public SMsgHdr hdr;
            public SChillerStat status;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SEncoderStat
        {
	        public Int32	info;
	        public Int32	warn;
	        public Int32	err;
	        public Int32	PG_cnt;
	        public Int32	fifoEmpty_PG;
	        public Int32	fifoEmpty_IGN;
	        public Int32	fifoEmpty_WND;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	        public Int32[]	corrRotPar;	// parameters for CORR_LINEAR
            public Int32 ampl_old;
            public Int32 ampl_new;
            public Int32 percentage;
            public UInt32 meters;
        }

		public const Byte SCL_READY         = 0x01;
		public const Byte SCL_CAL_ZERO      = 0x02;
		public const Byte SCL_CAL_SENSOR1   = 0x03;
		public const Byte SCL_CAL_SENSOR2   = 0x04;
		public const Byte SCL_CAL_SENSOR3   = 0x05;
		public const Byte SCL_CAL_OK        = 0x06;

		public const Byte SCL_SENS_READY    = 0x01;
		public const Byte SCL_SENS_BUSY     = 0x02;
		public const Byte SCL_SENS_OK       = 0x03;
		public const Byte SCL_SENS_ERROR    = 0x04;
        public const Byte SCL_SENS_OK_ERR   = 0x05;

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SScalesMsg
        {
        	public SMsgHdr			hdr;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 18)]
	        public Int32[]			val;
        };

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SFluidCtrlCmd
        {
	        public SMsgHdr          hdr;
	        public Int32			no;
	        public EFluidCtrlMode   ctrlMode;
        };

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SHeadEEpromInfo
        {
            //--- fuji eeprom ---------------------------------
            public UInt32 serialNo;
            public Byte week;
            public Byte year;
            public Byte flowResistance;
            public Byte straightness;
            public Int16 uniformity;
            public UInt16 voltage;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
            public UInt16[] dropSize;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
            public UInt16[] badJets;
        };

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SHeadEEpromMvt
        {
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
	        public Int16[]			disabledJets;	
	        public Int16	        clusterNo;
	        public Int32	        printed_ml;
        };

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SHeadStat
        {
            public UInt32		    info;
	        public UInt32		    warn;
	        public UInt32		    err;

	        //-- job info ------------------------------------
	        public UInt64	        dotCnt;	// printed drops since last reset
            public UInt32	        imgInCnt;
            public UInt32	        imgBuf;
            public UInt32       	encPos;
            public UInt32       	encPgCnt;
	        public UInt32	        printGoCnt;
	        public UInt32	        printDoneCnt;

	        //--- ink system ---------------------------------
	        public UInt32			tempHead;
	        public UInt32			tempCond;
	        public Int32			presIn;
	        public Int32			presIn_max;
	        public Int32			presIn_diff;
	        public Int32			presOut;
	        public Int32			presOut_diff;
            public Int32			meniscus;
            public Int32			meniscus_diff;
            public Int32			controller_offset;

	        public UInt32			pumpSpeed;
	        public UInt32			pumpFeedback;
	        public UInt32			printingSeconds;
            public Int32			presIn_0out;
	        public EFluidCtrlMode   ctrlMode;

            SHeadEEpromInfo         eeprom;
        	SHeadEEpromMvt	        eeprom_mvt;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SHeadBoardStat
        {
            public UInt32		boardNo;

	        //--- rom values, stored in head board ----
	        public UInt64		macAddr;
	        public UInt64		serialNo;
	        SVersion	hwVersion;
	        SVersion	swVersion;
	        SVersion	fpgaVersion;
	        SVersion	niosVersion;

	        //--- values stored in head-board (no reset) ---------
	        public UInt32		clusterNo;	
	        public UInt32		clusterTime; 
	        public UInt32		machineMeters; 

            public Int32        tempFpga;
            public Int32        flow;
 
            //--- warnings/errors ----------------
            public UInt32		info;
	        public UInt32		warn;
	        public UInt32		err;

	        public UInt16		headCnt;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	        public SHeadStat[]  head;

	        //--- job info -------------------------
	        public UInt32		encoderPos;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SHeadBoardStatMsg
        {
            public SMsgHdr          hdr;
            public SHeadBoardStat   stat;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SStepperMotor
        {
            public UInt32		state;
            //    #define MOTOR_STATE_UNDEF		0
		    //    #define MOTOR_STATE_IDLE		1	
		    //    #define MOTOR_STATE_MOVING_FWD	2
		    //    #define MOTOR_STATE_MOVING_BWD	3
		    //    #define MOTOR_STATE_BLOCKED		4		
            public Int32		motor_pos;
            public Int32		encoder_pos;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SStepperStat
        {
	        public SMsgHdr      hdr;
	        public UInt64		macAddr;
	        public UInt64		serialNo;

	        SVersion	        swVersion;
	        SVersion	        fpgaVersion;

            public UInt32		info;
	        public UInt32		warn;
	        public UInt32		err;

	        public Int32		posX;
	        public Int32		posY;
	        public Int32		posZ;

            public Int32        adjustmentProgress;
            public Int32        state;

            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
            public Int32[]       alive;

            public Int32			inputs;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 5)]
	        public SStepperMotor[]	motor;

            public Int32		set_io_cnt;

            public EFluidCtrlMode	ctrlModeCfg;
	        public EFluidCtrlMode	ctrlModeStat;	
        };

        //--- CLEAF Orders ------------------------------------------------------
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SCleafFlexo
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst =32)]
	        public string       lacCode;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst =32)]
	        public string       anilox;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst =32)]
	        public string       rubberRoll;
        };

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SCleafOrder
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst =32)]
	        public string       id;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst =32)]
	        public string       code;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst =32)]
	        public string       absCode;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst =32)]
	        public string       material;
            public Int32        speed;
            public Int32        length;
            public Double       printHeight;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst =32)]
            public string       printFile;
	        public Int32	    gloss;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst =256)]
            public string       note;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	        public SCleafFlexo[]	flexo;
        };

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SCleafProduction
        {
	        public Int32        produced;
	        public Int32        waste;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	        public Int32[]	inkUsed;
        };

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SCleafRoll	
        {
	        public Int32        no;
	        public double       length;
	        public Int32        quality;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst =64)]
            public string       user;
        };

    }
}
