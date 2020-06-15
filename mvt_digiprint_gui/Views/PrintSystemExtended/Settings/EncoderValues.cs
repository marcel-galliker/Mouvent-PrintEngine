using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.ComponentModel;

namespace RX_DigiPrint.Views.PrintSystemExtendedView
{
    public partial class EncoderValues : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        public EncoderValues(int no)
        {
            if (no < RxGlobals.Encoder.Length)
            {
                AmplNew = RxGlobals.Encoder[no].AmplNew;
                AmplOld = RxGlobals.Encoder[no].AmplOld;
                Meters = RxGlobals.Encoder[no].Meters;
                IsValid = RxGlobals.Encoder[no].IsValid;
            }
            else
            {
                AmplNew = (int)TcpIp.INVALID_VALUE;
                AmplOld = (int)TcpIp.INVALID_VALUE;
                Meters = 0;
                IsValid = false;
            }
        }

        private int _AmplOld;
        public int AmplOld
        {
            get { return _AmplOld; }
            set { _AmplOld = value; OnPropertyChanged("AmplOld"); }
        }

        private int _AmplNew;
        public int AmplNew
        {
            get { return _AmplNew; }
            set { _AmplNew = value; OnPropertyChanged("AmplNew"); }
        }

        private UInt32 _Meters;
        public UInt32 Meters
        {
            get { return _Meters; }
            set
            {
                _Meters = value;
                OnPropertyChanged("Meters");
                IsValid = (_Meters >= 50);
            }
        }
        private bool _IsValid;
        public bool IsValid
        {
            get { return _IsValid; }
            set { _IsValid = value; OnPropertyChanged("IsValid"); }
        }

    }
}
