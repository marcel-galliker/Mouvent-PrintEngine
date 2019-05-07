using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
    public class Encoder : RxBindable
    {
        private int _no;

        //--- constructor ---
        public Encoder(int no)
        {
            _no = no;
            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
            _printertype_changed();
        }

        //--- PrintSystem_PropertyChanged ----------------------------------
        private void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("PrinterType")) _printertype_changed();
        }

        //--- _printertype_changed --------------------
        void _printertype_changed()
        {
            Enabled = (RxGlobals.PrintSystem.PrinterType!=EPrinterType.printer_undef) && !RxGlobals.PrintSystem.IsScanning;
        }

        //--- Property Enabled ---------------------------------------
        private bool _Enabled=true;
        public bool Enabled
        {
            get { return _Enabled; }
            set { SetProperty(ref _Enabled, value); }
        }

        //--- Property IsValid ---------------------------------------
        private bool _IsValid=false;
        public bool IsValid
        {
            get { return _IsValid; }
            set { SetProperty(ref _IsValid, value); }
        }
        
        //--- Property AmplOld ---------------------------------------
        private int _AmplOld = (int)TcpIp.INVALID_VALUE;
        public int AmplOld
        {
            get { return _AmplOld; }
            set { SetProperty(ref _AmplOld, value); }
        }

        //--- Property AmplNew ---------------------------------------
        private int _AmplNew = (int)TcpIp.INVALID_VALUE;
        public int AmplNew
        {
            get { return _AmplNew; }
            set { SetProperty(ref _AmplNew, value); }
        }

        //--- Property Percentage ---------------------------------------
        private int _Percentage = (int)TcpIp.INVALID_VALUE;
        public int Percentage
        {
            get { return _Percentage; }
            set { SetProperty(ref _Percentage, value); }
        }

        //--- Property Meters ---------------------------------------
        private UInt32 _Meters;
        public UInt32 Meters
        {
            get { return _Meters; }
            set 
            { 
                if (SetProperty(ref _Meters, value))
                    IsValid = (_Meters>=50);
            }
        }
              
        //--- Request ---------------------------------
        public void Request()
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ENCODER_STAT);
        }

        //--- Save ---------------------------------
        public void Save()
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ENCODER_SAVE_PAR+(uint)_no);
        }

        //--- Update -----------------------------------
        public void Update(TcpIp.SEncoderStat msg)
        {
            AmplOld     = msg.ampl_old;
            AmplNew     = msg.ampl_new;
            Percentage  = msg.percentage;
            Meters      = msg.meters;
        }
    }
}
