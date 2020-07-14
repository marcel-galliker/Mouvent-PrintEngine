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
    public class TestTableStatus : RxBindable
    {
        private StepperMotor[] _Motors = new StepperMotor[5];

        //--- creator ----------------------
        public TestTableStatus()
        {
            int i;
            for (i=0; i<_Motors.Length; i++) _Motors[i]=new StepperMotor(){No=i};
            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
        }

        void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_LB701) cmd_enabled=cap_enabled=true;
        }
       
        //--- Property RefDone ---------------------------------------
        private bool _RefDone;
        public bool RefDone
        {
            get { return _RefDone; }
            set { SetProperty(ref _RefDone, value); }
        }

        //--- Property Mooving ---------------------------------------
        private bool _Moving;
        public bool Moving
        {
            get { return _Moving; }
            set { SetProperty(ref _Moving, value); }
        }
        
        //--- Property PosX ---------------------------------------
        private Int32 _PosX;
        public Int32 PosX
        {
            get { return _PosX; }
            set { SetProperty(ref _PosX, value); }
        }

        //--- Property PosY ---------------------------------------
        private Int32[] _PosY;
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

        //--- Property RobotUsed ---------------------------------------
        private bool _RobotUsed;
        public bool RobotUsed
        {
            get { return _RobotUsed; }
            set { SetProperty(ref _RobotUsed, value); }
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

        //--- Property cmd_enabled ---------------------------------------
        private bool _cmd_enabled;
        public bool cmd_enabled
        {
            get { return _cmd_enabled; }
            set { SetProperty(ref _cmd_enabled, value); }
        }
        
        //--- Property cmd_enabled ---------------------------------------
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
        
        //--- Update -----------------------------------
        public void Update(TcpIp.SStepperStat msg)
        {
            RefDone   = (msg.info & 0x00000001)!=0;
            Moving    = (msg.info & 0x00000002)!=0;
            UV_On     = (msg.info & 0x00000004)!=0;
            UV_Ready  = (msg.info & 0x00000008)!=0;
            Z_in_ref  = (msg.info & 0x00000010)!=0;
            Z_in_print= (msg.info & 0x00000020)!=0;
            Z_in_cap  = (msg.info & 0x00000040)!=0;
            X_in_cap  = (msg.info & 0x00000100)!=0 || RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_LB701;
            X_in_ref  = (msg.info & 0x00000200) != 0;
            CoverOpen = (msg.info & 0x00001000)!=0;
            
            HeadUpInput_0 = (msg.info & 0x00040000)!=0;
            HeadUpInput_1 = (msg.info & 0x00080000)!=0;
            HeadUpInput_2 = (msg.info & 0x00100000)!=0;
            HeadUpInput_3 = (msg.info & 0x00200000)!=0;

            DripPans_InfeedUP       = (msg.info & 0x10000000) != 0;
            DripPans_InfeedDOWN     = (msg.info & 0x20000000) != 0;
            DripPans_OutfeedUP      = (msg.info & 0x40000000) != 0;
            DripPans_OutfeedDOWN    = (msg.info & 0x80000000) != 0;
            RobotUsed               = (msg.robot_used!=0);

            drip_pans_enabled = Z_in_ref;

            PosX    = msg.posX;
            PosZ    = msg.posZ;
            for (int i = 0; i < PosY.Length; i++)
            {
                PosY[i] = msg.posY[i];
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
                cap_enabled = RefDone ;
                capDP803_enabled = false;
            }


            if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_cleaf) cmd_enabled = RefDone && DripPans_InfeedDOWN && DripPans_OutfeedDOWN && !DripPans_InfeedUP && !DripPans_OutfeedUP;
            //---- CAPPING ----
            else if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_DP803) cmd_enabled = RefDone && X_in_ref;
            //---- END OF CAPPING ----
            else cmd_enabled = RefDone;// && !Z_in_cap;

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
