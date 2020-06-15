using RX_DigiPrint.Models;
using System;
using System.ComponentModel;

namespace RX_DigiPrint.Views.PrintSystemExtendedView
{
    public partial class OffsetValues : INotifyPropertyChanged
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

        public OffsetValues()
        {
            OffsetIncPerMeter = RxGlobals.PrintSystem.OffsetIncPerMeter;
            OffsetStep = RxGlobals.PrintSystem.OffsetStep;
            OffsetAngle = RxGlobals.PrintSystem.OffsetAngle;
            Overlap = RxGlobals.PrintSystem.Overlap;
            OffsetIncPerMeterVerso = RxGlobals.PrintSystem.OffsetIncPerMeterVerso;
            OffsetVerso = RxGlobals.PrintSystem.OffsetVerso;
            ManualFlightTimeComp = RxGlobals.PrintSystem.ManualFlightTimeComp;
        }

        private int _OffsetIncPerMeter;
        public int OffsetIncPerMeter
        {
            get { return _OffsetIncPerMeter; }
            set { _OffsetIncPerMeter = value; OnPropertyChanged("OffsetIncPerMeter"); }
        }

        private Int32 _OffsetStep;
        public int OffsetStep
        {
            get { return _OffsetStep; }
            set { _OffsetStep = value; OnPropertyChanged("OffsetStep"); }
        }

        private Int32 _OffsetAngle;
        public int OffsetAngle
        {
            get { return _OffsetAngle; }
            set { _OffsetAngle = value; OnPropertyChanged("OffsetAngle"); }
        }

        private bool _Overlap;
        public bool Overlap
        {
            get { return _Overlap; }
            set { _Overlap = value; OnPropertyChanged("Overlap"); }
        }

        private int _OffsetIncPerMeterVerso;
        public int OffsetIncPerMeterVerso
        {
            get { return _OffsetIncPerMeterVerso; }
            set { _OffsetIncPerMeterVerso = value; OnPropertyChanged("OffsetIncPerMeterVerso"); }
        }

        private Int32 _OffsetVerso;
        public int OffsetVerso
        {
            get { return _OffsetVerso; }
            set { _OffsetVerso = value; OnPropertyChanged("OffsetVerso"); }
        }

        private double _ManualFlightTimeComp;
        public double ManualFlightTimeComp
        {
            get { return _ManualFlightTimeComp; }
            set { _ManualFlightTimeComp = value; OnPropertyChanged("ManualFlightTimeComp"); }
        }
    }
}
