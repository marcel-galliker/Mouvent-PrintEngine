using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Shell;

namespace RxRexrothGui.Services
{
    public class RxRexroth
    {
        public struct SSystemInfo
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
	        public string versionHardware;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
	        public string versionFirmware;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
	        public string versionLogic;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
	        public string versionBsp;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
	        public string versionMlpi;		// MLPI server core

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
	        public string serialNo;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
	        public string hardwareDetails;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
	        public string localBusConfig;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
	        public string modulBusConfig;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
	        public string modulHwDetails;

            public float tempAct;
            public float tempMax;

            //--- LOG tests --------------------------------
            public UInt32 indexOldest;
            public UInt32 indexNewest;
        };

        public struct SSystemTime
        {
          public UInt16  year;           //! Year, 2000..2099
          public UInt16  month;          //! Month, 1..12: 1==January, ..., 12==December
          public UInt16  day;            //! Day of month, 1..31
          public UInt16  hour;           //! Hour after midnight, 0..23
          public UInt16  minute;         //! Minute after hour, 0..59
          public UInt16  second;         //! Seconds after minute, 0..59
          public UInt16  milliSecond;    //! Milliseconds after second, 0..999
          public UInt16  microSecond;    //! Microseconds after millisecond, 0..999
          public UInt16  dayOfWeek;      //! Day of Week, 1..7: 1==Monday, ..., 7==Sunday
          public UInt16  dayOfYear;      //! Day of Year, 1..366: 1==January 1, 365/366== December 31
        };

        public enum EnPlcLogState : int
        {
	        undef	  = 0x00,
	        passive   = 0x00,   //!< diagnosis entry is passive.
	        active    = 0x01,   //!< diagnosis entry is active.
	        reset     = 0x02,   //!< diagnosis entry is reset.
	        message   = 0x04,
        };

        public struct SPlcLogItem
        {
	        public int no;
	        public EnPlcLogState state;
	        public UInt32 errNo;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
	        public string date;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
	        public string text;
        };

        public struct SPlcLogItemMsg
        {
            public TcpIp.SMsgHdr hdr;
            public SPlcLogItem   item;
        };

        [UnmanagedFunctionPointerAttribute(CallingConvention.StdCall)]
        public delegate void connected_callback();

        [UnmanagedFunctionPointerAttribute(CallingConvention.StdCall)]
        public delegate void error_callback();

        //--- Interface to RIP DLL --------------------------------

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int REX_Connect(string ipAddress, connected_callback onConnected, error_callback onError);
        
        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int REX_Load		     (string filepath);

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int REX_Save		     (string filepath, string filter);


        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool LC_GetVar (int no, StringBuilder name, StringBuilder value);

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool LC_GetValue (int no, StringBuilder value);

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool LC_GetValueByName (string name, StringBuilder value);

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool LC_SetValueByName (string name, string value);

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SYS_GetDisplayText (ref UInt32 errNo, StringBuilder text, int size, ref UInt32 mode);

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SYS_GetLogEntry	 (int no, ref UInt32 errNo, StringBuilder date, StringBuilder text, int size);

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool PAR_GetName	(int no, StringBuilder device, StringBuilder sidn, StringBuilder name, StringBuilder value, int size);
        
        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool PAR_GetValue	(string sidn, StringBuilder value, int size);

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool PAR_SetValue	(string sidn, string value);


        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SYS_GetInfo    (ref SSystemInfo info);

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SYS_GetTime	(ref SSystemTime time);

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SYS_SetTime	(ref SSystemTime time);

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SYS_ResetError	();

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SYS_SetMode	(UInt32 mode);

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SYS_Reboot	    ();

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SYS_CleanUp	();

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int APP_Start	    ();

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int APP_Stop	    ();

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int APP_Reset	    ();

        [DllImport("RX-Rexroth.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int APP_Download	(string filepath);
    }
}
