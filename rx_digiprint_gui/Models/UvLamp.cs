using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace RX_DigiPrint.Models
{
    public class UvLamp : RxBindable
    {
        //--- constructor -----------------
        public UvLamp()
        {
            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
            RxGlobals.Timer.TimerFct += Timer;
            RxGlobals.Plc.PropertyChanged += Plc_PropertyChanged;
        }

        //--- ShutDown ---------------------------------
        public void ShutDown()
        {
            RxGlobals.Timer.TimerFct -= Timer;
        }

        //--- PrintSystem_PropertyChanged -----------------------------------
        void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("PrinterType"))
            {
                switch (RxGlobals.PrintSystem.PrinterType)
                {
                    case Services.EPrinterType.printer_LB701:      Visible = Visibility.Visible; break;
                    case Services.EPrinterType.printer_LB702_UV:   Visible = Visibility.Visible; break;
                //  case Services.EPrinterType.printer_LH702:      Visible = Visibility.Visible; break;
                    default: Visible = Visibility.Collapsed; break;
                }
            }
        }

        //--- Property Vislible ---------------------------------------
        private Visibility _Visible = Visibility.Collapsed;
        public Visibility Visible
        {
            get { return _Visible; }
            set { SetProperty(ref _Visible, value); }
        }

        //--- SwitchOn ------------------------------------------------------
        public void SwitchOn()
        {
            if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_test_table)
                RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ENCODER_UV_ON);
            else
               RxGlobals.Plc.SetVar("CMD_UV_LAMPS_ON", 1);
        }

        //--- SwitchOff ------------------------------------------------------------
        public void SwitchOff()
        {
            if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_test_table)
                RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ENCODER_UV_OFF);
            else    
                RxGlobals.Plc.SetVar("CMD_UV_LAMPS_OFF", 1);
        }

        //--- Property On ---------------------------------------
        private bool _On;
        public bool On
        {
            get { return _On; }
            set { SetProperty(ref _On, value); }
        }

        //--- Property Ready ---------------------------------------
        private bool _Ready;
        public bool Ready
        {
            get { return _Ready; }
            set { SetProperty(ref _Ready, value); }
        }

        //--- Property Busy ---------------------------------------
        private bool _Busy;
        public bool Busy
        {
            get { return _Busy; }
            set { SetProperty(ref _Busy, value); }
        }

        //--- Property Time ---------------------------------------
        private int _Time;
        public int Time
        {
            get { return _Time; }
            set 
            { 
                if (SetProperty(ref _Time, value))
                {
                    if (value==0) 
                    {
                        TimeStr=ActionStr="";
                        Busy = false;
                    }
                    else
                    {
                        if (value>0) TimeStr = ActionStr="ON";
                        else         TimeStr = ActionStr="OFF"; 
                        TimeStr = string.Format("{0}s", Math.Abs(value));
                        Busy = true;
                    }
                }
            }
        }

        //--- Property ActionStr ---------------------------------------
        private string _ActionStr;
        public string ActionStr
        {
            get { return _ActionStr; }
            set { SetProperty(ref _ActionStr, value); }
        }
        
        //--- Property _TimeStr ---------------------------------------
        private string _TimeStr;
        public string TimeStr
        {
            get { return _TimeStr; }
            set { SetProperty(ref _TimeStr, value); }
        }
                
        //--- Timer ---------------------------------------------
        private void Timer(int no)
        {            
            if (!RxGlobals.Plc.Connected) return;
            string str;
            str = "Application.GUI_00_001_Main"+"\n"
                + "PAR_UV_LAMP_1_MODE"+"\n"
                + "PAR_UV_LAMP_2_MODE"+"\n"
                + "PAR_UV_LAMP_3_MODE"+"\n"
                + "PAR_UV_LAMP_4_MODE"+"\n"
                + "PAR_UV_LAMP_5_MODE"+"\n";
//                + "STA_UV_LAMP_1_TIMER"+"\n";

            RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_GET_VAR, str);

            str = "Application.GUI_00_001_Main"+"\n"
                + "STA_UV_POWER_ON"+"\n"
                + "STA_UV_LAMPS_READY"+"\n"
                + "STA_UV_LAMP_1_TIMER"+"\n";

            RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_GET_VAR, str);
        }

        //--- Plc_PropertyChanged ----------------------------
        private void Plc_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            string on=RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_UV_POWER_ON");
            On = (on!=null) && on.Equals("TRUE");

            string ready=RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_UV_LAMPS_READY");
            Ready = (ready!=null) && ready.Equals("TRUE");

            double time  = Rx.StrToDouble(RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_UV_LAMP_1_TIMER"));

            Time = (int)time;
        }

    }
}
