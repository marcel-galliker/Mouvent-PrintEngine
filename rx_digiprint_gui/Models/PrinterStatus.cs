using RX_Common;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.UserControls;
using System;
using System.Collections.Generic;

namespace RX_DigiPrint.Models
{
    public class PrinterStatus : RxBindable
    {
        
        public delegate void ErrorTypeChanged(ELogType errorType);

        public ErrorTypeChanged ErrorTypeChangedEvent = null;

        //--- constructor --------------------------------
        public PrinterStatus()
        {
            
        }

        //--- Upadte -------------------------------------
        public void Upadte(TcpIp.SPrinterStatusMsg msg)
        {
            PrintState          = msg.status.printState;
            TestMode            = msg.status.testMode;
            LogType             = (ELogType)msg.status.error;
            AllInkSupliesOff    = msg.status.inkSupilesOff!=0;
            AllInkSupliesOn     = msg.status.inkSupilesOn!=0;
            MaxSpeeds           = msg.status.maxSpeed;
            DataReady           = (msg.status.dataReady!=0);
            ExternalData        = (msg.status.externalData!=0);
        }
        
        //--- Printing ------------------------------------
        private static EPrintState _printstate=EPrintState.ps_off;
        public EPrintState PrintState
        {
            get { return _printstate; }
            set 
            {
                if (SetProperty(ref _printstate, value))
                {
                    IsOff = (value==EPrintState.ps_off || value==EPrintState.ps_ready_power);
                    switch(value)
                    {
                        case EPrintState.ps_off:        Console.WriteLine("ps_off");      break;
                        case EPrintState.ps_printing:   Console.WriteLine("ps_printing"); break;
                        case EPrintState.ps_stopping:   Console.WriteLine("ps_stopping"); break;
                        case EPrintState.ps_undef:      break;
                    } 
                    if (_image!=null) _image.Dispatcher.Invoke(()=>_image.Printing=(value==EPrintState.ps_printing || value==EPrintState.ps_stopping));
                }
            }
        }

        //--- Property IsOff ---------------------------------------
        private bool _IsOff=true;
        public bool IsOff
        {
            get { return _IsOff; }
            set { SetProperty(ref _IsOff, value); }
        }
        
        //--- Property AllInkSupliesOn ---------------------------------------
        private bool _AllInkSupliesOn=false;
        public bool AllInkSupliesOn
        {
            get { return _AllInkSupliesOn; }
            set { SetProperty(ref _AllInkSupliesOn, value); }
        }        

        //--- Property AllInkSupliesOff ---------------------------------------
        private bool _AllInkSupliesOff=true;
        public bool AllInkSupliesOff
        {
            get { return _AllInkSupliesOff; }
            set { SetProperty(ref _AllInkSupliesOff, value); }
        }        

        //--- Property TestMode ---------------------------------------
        private UInt32 _TestMode;
        public UInt32 TestMode
        {
            get { return _TestMode; }
            set { SetProperty(ref _TestMode, value); }
        }

        //--- Property ExternalData ---------------------------------------
        private bool _ExternalData=false;
        public bool ExternalData
        {
            get { return _ExternalData; }
            set { SetProperty(ref _ExternalData, value); }
        }
        
        //--- Property MaxSpeed ---------------------------------------
        private UInt32[] _MaxSpeed={120, 120,100,60};
        public UInt32[] MaxSpeeds
        {
            get { return _MaxSpeed; }
            set 
            { 
                SetProperty(ref _MaxSpeed, value);
                /*
                int i;
                List<EN_SpeedList> speeds = new List<EN_SpeedList>();
                for (i=0; i<3; i++)
                {
                    speeds.Add(new EN_SpeedList(_MaxSpeed[i]));
                }
                RxGlobals.PrintSystem.SpeedList = speeds;
                 * */
            }
        }

        public UInt32 MaxSpeed(int dropSize)
        {
            if (dropSize>0 && dropSize<=3) return _MaxSpeed[dropSize];
            return _MaxSpeed[3];
        }

        //--- Property DataReady ---------------------------------------
        private bool _DataReady;
        public bool DataReady
        {
            get { return _DataReady; }
            set { SetProperty(ref _DataReady, value); }
        }
        
        //--- Error ---------------------------------------
        private static ELogType _LogType=0;
        public ELogType LogType
        {
            get { return _LogType; }
            set 
            { 
                if (SetProperty(ref _LogType, value))
                {
                    if (ErrorTypeChangedEvent!=null) RxBindable.Invoke(()=>ErrorTypeChangedEvent(value));
                } 
            }
        }

        //--- _SetBnState ----------------------------------------------------
        private void _SetBnState(ref RxBtDef.EButtonState state, bool isEnabled, bool isCheced, double size)
        {
            RxBtDef.EButtonState st=0;
            if (isEnabled)  st |= RxBtDef.EButtonState.isEnabled;
            if (isCheced)   st |= RxBtDef.EButtonState.isChecked;
            if (size>32)    st |= RxBtDef.EButtonState.isLarge;
            SetProperty(ref state, st);
        }

        //--- Image ---------------------------------------
        private static PrinterImage _image;
        public PrinterImage  Image
        {
            get { return _image; }
            set { SetProperty(ref _image, value); }
        }
    }
}
