using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;

namespace RX_DigiPrint.Models
{
    public class StepperStatus : RxBindable
    {
        private StepperMotor[] _Motors = new StepperMotor[5];

        //--- creator ----------------------
        public StepperStatus(int n)
        {
            No=n;
            for (int i=0; i<_Motors.Length; i++) _Motors[i]=new StepperMotor(){No=i};
        }

        //--- Property No ---------------------------------------
        private int _No;
        public int No
        {
            get { return _No; }
            set { SetProperty(ref _No,value); }
        }

        //--- Property RefDone ---------------------------------------
        private bool _Connected;
        public bool Connected
        {
            get { return _Connected; }
            set { SetProperty(ref _Connected, value); }
        }

        //--- Property RefDone ---------------------------------------
        private bool _RefDone;
        public bool RefDone
        {
            get { return _RefDone; }
            set { SetProperty(ref _RefDone, value); }
        }

        //--- Property RefDone ---------------------------------------
        private bool _RobotRefDone;
        public bool RobotRefDone
        {
            get { return _RobotRefDone; }
            set { SetProperty(ref _RobotRefDone, value); }
        }

        //--- Property CmdRunning ---------------------------------------
        private uint _CmdRunning=TcpIp.INVALID_VALUE;
        public uint CmdRunning
        {
            get { return _CmdRunning; }
            set { SetProperty(ref _CmdRunning,value); }
        }

        //--- Property CmdRunning ---------------------------------------
        private uint _ClnCmdRunning=TcpIp.INVALID_VALUE;
        public uint ClnCmdRunning
        {
            get { return _ClnCmdRunning; }
            set { SetProperty(ref _ClnCmdRunning,value); }
        }

        //--- Property Mooving ---------------------------------------
        private bool _Moving;
        public bool Moving
        {
            get { return _Moving; }
            set { SetProperty(ref _Moving, value); }
        }
        //--- Property Mooving ---------------------------------------
        private bool _RobMoving;
        public bool RobMoving
        {
            get { return _RobMoving; }
            set { SetProperty(ref _RobMoving, value); }
        }

		//--- Property ScrewerReady ---------------------------------------
		private bool _ScrewerReady;
		public bool ScrewerReady
		{
			get { return _ScrewerReady; }
			set { SetProperty(ref _ScrewerReady,value); }
		}

		//--- Property Screwing ---------------------------------------
		private bool _Screwing;
		public bool Screwing
		{
			get { return _Screwing; }
			set { SetProperty(ref _Screwing,value); }
		}

		//--- Property _AdjustDoneCnt ---------------------------------------
		private int _AdjustDoneCnt;
		public int AdjustDoneCnt
		{
			get { return _AdjustDoneCnt; }
			set {
                    if (SetProperty(ref _AdjustDoneCnt, value)) 
                        RxGlobals.Events.AddItem(new LogItem(string.Format("_AdjustDoneCnt={0}", _AdjustDoneCnt)));
                }
		}

		//--- Property ScrewBlocked ---------------------------------------
		private bool _ScrewedOk;
		public bool ScrewedOk
		{
			get { return _ScrewedOk; }
			set { SetProperty(ref _ScrewedOk,value); }
		}

		//--- Property PosX ---------------------------------------
		private Int32 _PosX;
        public Int32 PosX
        {
            get { return _PosX; }
            set { SetProperty(ref _PosX, value); }
        }

        //--- Property PosY ---------------------------------------
        private Int32[] _PosY=new Int32[4];
        public Int32[] PosY
        {
            get { return _PosY; }
            set { SetProperty(ref _PosY, value); }
        }

        //--- Property PosZ ---------------------------------------
        private Int32 _PosZ;
        public Int32 PosZ
        {
            get { return _PosZ; }
            set { SetProperty(ref _PosZ, value); }
        }

        //--- LiftState ---------------------------------------
        private string _LiftState;
        public string LiftState
        {
            get { return _LiftState; }
            set { SetProperty(ref _LiftState, value); }
        }

        //--- RoboState ---------------------------------------
        private string _RoboState;
        public string RoboState
        {
            get { return _RoboState; }
            set { SetProperty(ref _RoboState, value); }
        }

        //--- Property Error ---------------------------------------
        private UInt32 _Error;
        public UInt32 Error
        {
            get { return _Error; }
            set { SetProperty(ref _Error, value); }
        }

        //--- Property UV_On ---------------------------------------
        private bool _UV_On;
        public bool UV_On
        {
            get { return _UV_On; }
            set 
            { 
                if (SetProperty(ref _UV_On, value)) 
                    UV_Busy= _UV_On && !_UV_Ready; 
            }
        }

        //--- Property UV_Ready ---------------------------------------
        private bool _UV_Ready;
        public bool UV_Ready
        {
            get { return _UV_Ready; }
            set 
            { 
                if (SetProperty(ref _UV_Ready, value)) 
                    UV_Busy= _UV_On && !_UV_Ready; 
            }
        }

        //--- Property UV_Busy ---------------------------------------
        private bool _UV_Busy=false;
        public bool UV_Busy
        {
            get { return _UV_Busy; }
            set { SetProperty(ref _UV_Busy, value); }
        }

        //--- Property _CoverOpen ---------------------------------------
        private bool _CoverOpen;
        public bool CoverOpen
        {
            get { return _CoverOpen; }
            set { SetProperty(ref _CoverOpen, value); }
        }

        //--- Property Z_in_ref ---------------------------------------
        private bool _Z_in_ref;
        public bool Z_in_ref
        {
            get { return _Z_in_ref; }
            set { SetProperty(ref _Z_in_ref, value); }
        }

        //--- Property Z_in_wash ---------------------------------------
        private bool _Z_in_wash;
        public bool Z_in_wash
        {
            get { return _Z_in_wash; }
            set { SetProperty(ref _Z_in_wash, value); }
        }

        //--- Property Z_in_up ---------------------------------------
        private bool _Z_in_up;
        public bool Z_in_up
        {
            get { return _Z_in_up; }
            set { SetProperty(ref _Z_in_up, value); }
        }

        //--- Property Z_in_print ---------------------------------------
        private bool _Z_in_print;
        public bool Z_in_print
        {
            get { return _Z_in_print; }
            set { SetProperty(ref _Z_in_print, value); }
        }

        //--- Property Z_in_cap ---------------------------------------
        private bool _Z_in_cap;
        public bool Z_in_cap
        {
            get { return _Z_in_cap; }
            set { SetProperty(ref _Z_in_cap, value); }
        }

        //--- Property X_in_cap ---------------------------------------
        private bool _X_in_cap;
        public bool X_in_cap
        {
            get { return _X_in_cap; }
            set { SetProperty(ref _X_in_cap, value); }
        }

        //--- Property X_in_ref ---------------------------------------
        private bool _X_in_ref;
        public bool X_in_ref
        {
            get { return _X_in_ref; }
            set { SetProperty(ref _X_in_ref, value); }
        }

		//--- Property ClnWashDone ---------------------------------------
		private bool _ClnWashDone;
		public bool ClnWashDone
		{
			get { return _ClnWashDone; }
			set { SetProperty(ref _ClnWashDone,value); }
		}

		//--- Property ClnVacuumDone ---------------------------------------
		private bool _ClnVacuumDone;
		public bool ClnVacuumDone
		{
			get { return _ClnVacuumDone; }
			set { SetProperty(ref _ClnVacuumDone,value); }
		}


		//--- Property ClnUsed ---------------------------------------
		private bool _ClnUsed;
        public bool ClnUsed
        {
            get { return _ClnUsed; }
            set { 
                    SetProperty(ref _ClnUsed, value); 
                    if (_ClnUsed) RxGlobals.PrintSystem.IsClnUsed=true;
                }
        }

        //--- Property ScrewerUsed ---------------------------------------
        private bool _ScrewerUsed;
        public bool ScrewerUsed
        {
            get { return _ScrewerUsed; }
            set { 
                    SetProperty(ref _ScrewerUsed, value); 
                    if (_ScrewerUsed) RxGlobals.PrintSystem.IsRobotConnected=true;
                }
        }

        //--- Property _HeadUpInput_0 ---------------------------------------
        private bool _HeadUpInput_0;
        public bool HeadUpInput_0
        {
            get { return _HeadUpInput_0; }
            set { SetProperty(ref _HeadUpInput_0, value); }
        }

        //--- Property _HeadUpInput_1 ---------------------------------------
        private bool _HeadUpInput_1;
        public bool HeadUpInput_1
        {
            get { return _HeadUpInput_1; }
            set { SetProperty(ref _HeadUpInput_1, value); }
        }

        //--- Property _HeadUpInput_2 ---------------------------------------
        private bool _HeadUpInput_2;
        public bool HeadUpInput_2
        {
            get { return _HeadUpInput_2; }
            set { SetProperty(ref _HeadUpInput_2, value); }
        }

        //--- Property _HeadUpInput_3 ---------------------------------------
        private bool _HeadUpInput_3;
        public bool HeadUpInput_3
        {
            get { return _HeadUpInput_3; }
            set { SetProperty(ref _HeadUpInput_3, value); }
        }
        
        //--- Property _cap_enabled ---------------------------------------
        private bool _cap_enabled;
        public bool cap_enabled
        {
            get { return _cap_enabled; }
            set { SetProperty(ref _cap_enabled, value); }   
        }

        //--- Property capDP803_enabled ---------------------------------------
        private bool _capDP803_enabled;
        public bool capDP803_enabled
        {
            get { return _capDP803_enabled; }
            set { SetProperty(ref _capDP803_enabled, value); }
        }

        //--- Property drip_pans_enabled ---------------------------------------
        private bool _drip_pans_enabled;
        public bool drip_pans_enabled
        {
            get { return _drip_pans_enabled; }
            set { SetProperty(ref _drip_pans_enabled, value); }
        }

        //--- Property DripPans_InfeedUP ---------------------------------------
        private bool _DripPans_InfeedUP;
        public bool DripPans_InfeedUP
        {
            get { return _DripPans_InfeedUP; }
            set { SetProperty(ref _DripPans_InfeedUP, value); }
        }

        //--- Property DripPans_InfeedDOWN ---------------------------------------
        private bool _DripPans_InfeedDOWN;
        public bool DripPans_InfeedDOWN
        {
            get { return _DripPans_InfeedDOWN; }
            set { SetProperty(ref _DripPans_InfeedDOWN, value); }
        }

        //--- Property DripPans_OutfeedUP ---------------------------------------
        private bool _DripPans_OutfeedUP;
        public bool DripPans_OutfeedUP
        {
            get { return _DripPans_OutfeedUP; }
            set { SetProperty(ref _DripPans_OutfeedUP, value); }
        }

        //--- Property DripPans_OutfeedDOWN ---------------------------------------
        private bool _DripPans_OutfeedDOWN;
        public bool DripPans_OutfeedDOWN
        {
            get { return _DripPans_OutfeedDOWN; }
            set { SetProperty(ref _DripPans_OutfeedDOWN, value); }
        }

        //--- Property Motors ---------------------------------------
        public StepperMotor[] Motors
        {
            get { return _Motors; }
            set { SetProperty(ref _Motors, value); }
        }

        //--- Property _TTS_Valve_C1_OFF ---------------------------------------
        private bool _TTS_Valve_C1_OFF;
        public bool TTS_Valve_C1_OFF
        {
            get { return _TTS_Valve_C1_OFF; }
            set { SetProperty(ref _TTS_Valve_C1_OFF, value); }
        }


        //--- Property _TTS_Valve_C2_OFF ---------------------------------------
        private bool _TTS_Valve_C2_OFF;
        public bool TTS_Valve_C2_OFF
        {
            get { return _TTS_Valve_C2_OFF; }
            set { SetProperty(ref _TTS_Valve_C2_OFF, value); }
        }


        //--- Property _TTS_Valve_C3_OFF ---------------------------------------
        private bool _TTS_Valve_C3_OFF;
        public bool TTS_Valve_C3_OFF
        {
            get { return _TTS_Valve_C3_OFF; }
            set { SetProperty(ref _TTS_Valve_C3_OFF, value); }
        }


        //--- Property _TTS_Valve_C4_OFF ---------------------------------------
        private bool _TTS_Valve_C4_OFF;
        public bool TTS_Valve_C4_OFF
        {
            get { return _TTS_Valve_C4_OFF; }
            set { SetProperty(ref _TTS_Valve_C4_OFF, value); }
        }

        //--- Property _TTS_Valve_C1_IPA ---------------------------------------
        private bool _TTS_Valve_C1_IPA;
        public bool TTS_Valve_C1_IPA
        {
            get { return _TTS_Valve_C1_IPA; }
            set { SetProperty(ref _TTS_Valve_C1_IPA, value); }
        }


        //--- Property _TTS_Valve_C2_IPA ---------------------------------------
        private bool _TTS_Valve_C2_IPA;
        public bool TTS_Valve_C2_IPA
        {
            get { return _TTS_Valve_C2_IPA; }
            set { SetProperty(ref _TTS_Valve_C2_IPA, value); }
        }


        //--- Property _TTS_Valve_C3_IPA ---------------------------------------
        private bool _TTS_Valve_C3_IPA;
        public bool TTS_Valve_C3_IPA
        {
            get { return _TTS_Valve_C3_IPA; }
            set { SetProperty(ref _TTS_Valve_C3_IPA, value); }
        }


        //--- Property _TTS_Valve_C4_IPA ---------------------------------------
        private bool _TTS_Valve_C4_IPA;
        public bool TTS_Valve_C4_IPA
        {
            get { return _TTS_Valve_C4_IPA; }
            set { SetProperty(ref _TTS_Valve_C4_IPA, value); }
        }

        //--- Property _TTS_Valve_C1_XL ---------------------------------------
        private bool _TTS_Valve_C1_XL;
        public bool TTS_Valve_C1_XL
        {
            get { return _TTS_Valve_C1_XL; }
            set { SetProperty(ref _TTS_Valve_C1_XL, value); }
        }


        //--- Property _TTS_Valve_C2_XL ---------------------------------------
        private bool _TTS_Valve_C2_XL;
        public bool TTS_Valve_C2_XL
        {
            get { return _TTS_Valve_C2_XL; }
            set { SetProperty(ref _TTS_Valve_C2_XL, value); }
        }


        //--- Property _TTS_Valve_C3_XL ---------------------------------------
        private bool _TTS_Valve_C3_XL;
        public bool TTS_Valve_C3_XL
        {
            get { return _TTS_Valve_C3_XL; }
            set { SetProperty(ref _TTS_Valve_C3_XL, value); }
        }


        //--- Property _TTS_Valve_C4_XL ---------------------------------------
        private bool _TTS_Valve_C4_XL;
        public bool TTS_Valve_C4_XL
        {
            get { return _TTS_Valve_C4_XL; }
            set { SetProperty(ref _TTS_Valve_C4_XL, value); }
        }

        //--- Property _TTS_Valve_C1_Waste ---------------------------------------
        private bool _TTS_Valve_C1_Waste;
        public bool TTS_Valve_C1_Waste
        {
            get { return _TTS_Valve_C1_Waste; }
            set { SetProperty(ref _TTS_Valve_C1_Waste, value); }
        }


        //--- Property _TTS_Valve_C2_Waste ---------------------------------------
        private bool _TTS_Valve_C2_Waste;
        public bool TTS_Valve_C2_Waste
        {
            get { return _TTS_Valve_C2_Waste; }
            set { SetProperty(ref _TTS_Valve_C2_Waste, value); }
        }


        //--- Property _TTS_Valve_C3_Waste ---------------------------------------
        private bool _TTS_Valve_C3_Waste;
        public bool TTS_Valve_C3_Waste
        {
            get { return _TTS_Valve_C3_Waste; }
            set { SetProperty(ref _TTS_Valve_C3_Waste, value); }
        }


        //--- Property _TTS_Valve_C4_Waste ---------------------------------------
        private bool _TTS_Valve_C4_Waste;
        public bool TTS_Valve_C4_Waste
        {
            get { return _TTS_Valve_C4_Waste; }
            set { SetProperty(ref _TTS_Valve_C4_Waste, value); }
        }

        //--- Update -----------------------------------
        public void Update(TcpIp.SStepperStat msg)
        {
            Connected = true;
            RefDone      = (msg.info & 0x00000001)!=0;
            RobotRefDone = (msg.robinfo & 0x00000001) != 0;
            Moving       = (msg.info & 0x00000002)!=0;
            RobMoving    = (msg.robinfo & 0x00000002) != 0;
            /*
            if (No==0) Console.WriteLine("ScrewerInfo: blockedleft={0}, blockedright={1}, ready={2}, ok={3}", 
                (msg.screwerinfo&0x00001000)!=0,
                (msg.screwerinfo&0x00002000)!=0,
                (msg.screwerinfo&0x00040000)!=0,
                (msg.screwerinfo&0x00000800)!=0);
            */
            ScrewedOk   = (msg.screwerinfo & 0x00000800) != 0;
            Screwing    = (msg.screwerinfo & 0x00000002) != 0;
            ScrewerReady= (msg.screwerinfo & 0x00004000) != 0; 
            AdjustDoneCnt = msg.adjustDoneCnt;
            UV_On     = (msg.info & 0x00000004)!=0;
            UV_Ready  = (msg.info & 0x00000008)!=0;
            Z_in_ref  = (msg.info & 0x00000010)!=0;
            Z_in_print= (msg.info & 0x00000020)!=0;
            Z_in_cap  = (msg.info & 0x00000040)!=0;
            Z_in_wash = (msg.info & 0x00000080)!=0;
            Z_in_up   = (msg.info & 0x00000100)!=0;
            X_in_cap  = (msg.info & 0x00000200)!=0 || RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_LB701;
            X_in_ref  = (msg.info & 0x00000400)!= 0;
            CoverOpen = (msg.info & 0x00001000)!=0;
            
            HeadUpInput_0 = (msg.info & 0x00040000)!=0;
            HeadUpInput_1 = (msg.info & 0x00080000)!=0;
            HeadUpInput_2 = (msg.info & 0x00100000)!=0;
            HeadUpInput_3 = (msg.info & 0x00200000)!=0;

            DripPans_InfeedUP       = (msg.info & 0x10000000) != 0;
            DripPans_InfeedDOWN     = (msg.info & 0x20000000) != 0;
            DripPans_OutfeedUP      = (msg.info & 0x40000000) != 0;
            DripPans_OutfeedDOWN    = (msg.info & 0x80000000) != 0;
            ClnUsed               = (msg.cln_used!=0) || RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_TX404;
            ScrewerUsed           = (msg.screwer_used!=0);

            ClnWashDone         = (msg.robinfo & 0x00020000) !=0;
            ClnVacuumDone       = (msg.robinfo & 0x00010000) !=0;

            TTS_Valve_C1_Waste = (msg.inkinfo & 0x00000001) != 0;
            TTS_Valve_C2_Waste = (msg.inkinfo & 0x00000002) != 0;
            TTS_Valve_C3_Waste = (msg.inkinfo & 0x00000004) != 0;
            TTS_Valve_C4_Waste = (msg.inkinfo & 0x00000008) != 0;
            TTS_Valve_C1_IPA = (msg.inkinfo & 0x00000010) != 0;
            TTS_Valve_C2_IPA = (msg.inkinfo & 0x00000020) != 0;
            TTS_Valve_C3_IPA = (msg.inkinfo & 0x00000040) != 0;
            TTS_Valve_C4_IPA = (msg.inkinfo & 0x00000080) != 0;
            TTS_Valve_C1_XL = (msg.inkinfo & 0x00000100) != 0;
            TTS_Valve_C2_XL = (msg.inkinfo & 0x00000200) != 0;
            TTS_Valve_C3_XL = (msg.inkinfo & 0x00000400) != 0;
            TTS_Valve_C4_XL = (msg.inkinfo & 0x00000800) != 0;
            TTS_Valve_C1_OFF = !(TTS_Valve_C1_IPA || TTS_Valve_C1_XL);
            TTS_Valve_C2_OFF = !(TTS_Valve_C2_IPA || TTS_Valve_C2_XL);
            TTS_Valve_C3_OFF = !(TTS_Valve_C3_IPA || TTS_Valve_C3_XL);
            TTS_Valve_C4_OFF = !(TTS_Valve_C4_IPA || TTS_Valve_C4_XL);

            drip_pans_enabled = Z_in_ref;
            
            CmdRunning = msg.cmdRunning;
            ClnCmdRunning = msg.clnCmdRunning;

            PosX    = msg.posX;
            PosZ    = msg.posZ;
            for (int i = 0; i < PosY.Length; i++)
            {
                PosY[i] = msg.posY[i];
            }

            //--- LiftState ------------------          
            {
                // why not using an ENUM for this?
                if (Moving)         LiftState = "Moving";
                else if (!RefDone)  LiftState = "Lift not ref";
                else if (Z_in_ref)  LiftState = "Lift Ref";
                else if (Z_in_up)   LiftState = "Lift Up";
                else if (Z_in_print)LiftState = "Lift Print";
                else if (Z_in_cap)  LiftState = "Lift Cap";
                else                LiftState = "Lift undef.";
            }

            //--- RoboState --------------------
            {
                if (RobMoving)          RoboState = "Moving";
                else if (!RobotRefDone) RoboState = "Robot not ref";
                else if (X_in_ref)      RoboState = "Robot in Ref";
                else if (X_in_cap)      RoboState = "Robot in Cap";
                else                    RoboState = "Robot undef.";
            }

            Error   = msg.err;

            //cap_enabled = RefDone && DripPans_InfeedUP && DripPans_OutfeedUP && (X_in_cap || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LB701 || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_cleaf);
            //---- CAPPING ----
            if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_DP803)
            {
                cap_enabled = RefDone && X_in_cap;
                capDP803_enabled = RefDone && Z_in_ref;
            }
            else if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_cleaf)
            {
                cap_enabled = RefDone && DripPans_InfeedUP && DripPans_OutfeedUP;
                capDP803_enabled = true;
            }
            //---- END OF CAPPING ----
            else
            {
                if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_LB702_WB)
                    cap_enabled = /*RefDone &*/ ClnUsed;
                else
                    cap_enabled = RefDone;  // Testtable?
                capDP803_enabled = false;
            }

            /*
            if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_cleaf) cmd_enabled = RefDone && DripPans_InfeedDOWN && DripPans_OutfeedDOWN && !DripPans_InfeedUP && !DripPans_OutfeedUP;
            //---- CAPPING ----
            else if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_DP803) cmd_enabled = RefDone && X_in_ref;
            //---- END OF CAPPING ----
            else cmd_enabled = RefDone;// && !Z_in_cap;
            */

            {
                int i;
                for (i=0; i<_Motors.Length; i++)
                {
                    _Motors[i].State        = msg.motor[i].state;
                    _Motors[i].MotorPos     = msg.motor[i].motor_pos/1000.0;
                    _Motors[i].EncoderPos   = msg.motor[i].encoder_pos/1000.0;
                    _Motors[i].EncoderColor = Math.Abs(msg.motor[i].motor_pos - msg.motor[i].encoder_pos) < 100 ? Colors.Transparent : Colors.Red;
                    _Motors[i].EndSwitch    = (msg.inputs & (1<<i))!=0;
                  //  _Motors[i].SendBt(null);
                }
            
            }
        }
    }
}
