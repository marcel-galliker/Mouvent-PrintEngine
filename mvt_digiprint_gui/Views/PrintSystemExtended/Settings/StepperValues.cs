using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System.ComponentModel;

namespace RX_DigiPrint.Views.PrintSystemExtendedView
{
    public partial class StepperValues : INotifyPropertyChanged
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

        private double _RefHeight;
        public double RefHeight
        {
            get { return _RefHeight; }
            set { _RefHeight = value; OnPropertyChanged("RefHeight"); }
        }
        private double _WipeHeight;
        public double WipeHeight
        {
            get { return _WipeHeight; }
            set { _WipeHeight = value; OnPropertyChanged("WipeHeight"); }
        }

        private double _CapHeight;
        public double CapHeight
        {
            get { return _CapHeight; }
            set { _CapHeight = value; OnPropertyChanged("CapHeight"); }
        }

        private double _AdjustPos;
        public double AdjustPos
        {
            get { return _AdjustPos; }
            set { _AdjustPos = value; OnPropertyChanged("AdjustPos"); }
        }

        private double _ManualFlightTimeComp;
        public double ManualFlightTimeComp
        {
            get { return _ManualFlightTimeComp; }
            set { _ManualFlightTimeComp = value; OnPropertyChanged("ManualFlightTimeComp"); }
        }

        private int _WipeDelay;
        public int WipeDelay
        {
            get { return _WipeDelay; }
            set { _WipeDelay = value; OnPropertyChanged("WipeDelay"); }
        }

        private int _WipeSpeed;
        public int WipeSpeed
        {
            get { return _WipeSpeed; }
            set { _WipeSpeed = value; OnPropertyChanged("WipeSpeed"); }
        }

        public TcpIp.SRobotOffsets[] Robot = new TcpIp.SRobotOffsets[4];

        public StepperValues()
        {
            RefHeight = RxGlobals.Stepper.RefHeight;
            WipeHeight = RxGlobals.Stepper.WipeHeight;
            CapHeight = RxGlobals.Stepper.CapHeight;
            AdjustPos = RxGlobals.Stepper.AdjustPos;
            ManualFlightTimeComp = RxGlobals.PrintSystem.ManualFlightTimeComp;
            WipeDelay = RxGlobals.Stepper.WipeDelay;
            WipeSpeed = RxGlobals.Stepper.WipeSpeed;
            for (int i = 0; i < RxGlobals.Stepper.Robot.Length; i++)
            {
                Robot[i].ref_height = RxGlobals.Stepper.Robot[i].ref_height;
                Robot[i].head_align = RxGlobals.Stepper.Robot[i].head_align;
                Robot[i].ref_height_front = RxGlobals.Stepper.Robot[i].ref_height_front;
                Robot[i].ref_height_back = RxGlobals.Stepper.Robot[i].ref_height_back;
                Robot[i].cap_height = RxGlobals.Stepper.Robot[i].cap_height;
            }
            RxGlobals.Stepper.PropertyChanged += Stepper_PropertyChanged;
        }

        void Stepper_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Robot"))
            {
                for (int i = 0; i < RxGlobals.Stepper.Robot.Length; i++)
                {
                    Robot[i].ref_height = RxGlobals.Stepper.Robot[i].ref_height;
                    Robot[i].head_align = RxGlobals.Stepper.Robot[i].head_align;
                    Robot[i].ref_height_front = RxGlobals.Stepper.Robot[i].ref_height_front;
                    Robot[i].ref_height_back = RxGlobals.Stepper.Robot[i].ref_height_back;
                    Robot[i].cap_height = RxGlobals.Stepper.Robot[i].cap_height;
                }
            }
        }
    }

}
