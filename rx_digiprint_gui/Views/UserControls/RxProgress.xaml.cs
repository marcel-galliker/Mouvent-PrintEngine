using RX_DigiPrint.Models;
using System;
using System.Windows.Controls;
using System.Windows.Media;

namespace RX_DigiPrint.Views.UserControls
{
    public partial class RxProgress : UserControl
    {
        private Brush   _Background = Brushes.Transparent;

        //--- Constructor --------------------------------------
        public RxProgress()
        {
            InitializeComponent();
            DataContext = this;
            RxGlobals.Timer.TimerFct  += Tick;
        }

        //--- Property ShowText ---------------------------------------
        public bool ShowText
        {
            set { if (!value) TextHeight.Height = new System.Windows.GridLength(0);}
        }
        
        //--- Property Color ---------------------------------------
        public Brush Color
        {
            get { return ProgressBack.Background; }
            set 
            { 
                ProgressBack.Background =value;
                ProgressStr.Background = value;
                SolidColorBrush brush = ProgressBack.Background as SolidColorBrush;
                int rgb= brush.Color.R+brush.Color.G+brush.Color.B;
                if ((brush.Color.B>100 && brush.Color.R<20 && brush.Color.G<20) 
                ||  (brush.Color.B<150 && brush.Color.R<150 && brush.Color.G<150)) ProgressBar.Foreground=Brushes.LightBlue;
                else ProgressBar.Foreground=Brushes.Blue;
            }
        }

        //--- Property TextColor ---------------------------------------
        public Brush TextColor
        {
            get { return ProgressStr.Foreground; }
            set { ProgressStr.Foreground = value; }
        }
        
        //--- Property Name ---------------------------------------
        public string Text
        {
            get { return ProgressText.Text; }
            set { ProgressText.Text = value; }
        }

        //--- Property Warn ---------------------------------------
        private bool _Warn;
        public bool Warn
        {
            get { return _Warn; }
            set { _Warn = value; }
        }

        //--- Property Err ---------------------------------------
        private bool _Err;
        public bool Err
        {
            get { return _Err; }
            set { _Err = value; }
        }
        
        //--- Property Value ---------------------------------------
        public UInt32 Value
        {
            get { return 0; }
            set 
            { 
                ProgressBar.Value = value; 
                if (_Err)      _Background = Brushes.Red;
                else if (_Warn)_Background = Brushes.Orange;
                else           _Background = Brushes.Transparent;
            }
        }

        //--- Property ValueStr ---------------------------------------
        public string ValueStr
        {
            set { ProgressStr.Text = value; }
        }        

        //--- Tick ----------------------
        void Tick(int tickNo)
        {
            if ((tickNo&1)==0) MainGrid.Background = _Background;
            else               MainGrid.Background = Brushes.Transparent;
        }
    }
}
