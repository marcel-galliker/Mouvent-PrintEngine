using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Globalization;
using System.Threading;

namespace RX_Common
{
    public partial class RxNumPad : Window, INotifyPropertyChanged
    {        
        private double  _OrgValue;
        private Point   _Position;
        private double  _ObjWidth=0;
        private double  _ObjHeight=0;

        //--- creator -----------------------------------
        public RxNumPad(TextBox obj)
        {
            InitializeComponent();
            this.DataContext = this;
            KeyDown += RxNumPad_KeyDown;  
            
            //--- timer as work around first touch behavior of ELO ------

            _OrgValue = Rx.StrToDouble(obj.Text);

            _Position  = obj.PointToScreen(new Point(0, 0));
            _ObjWidth  = obj.ActualWidth;
            _ObjHeight = obj.ActualHeight;

            RxScreen screen =new RxScreen();
            screen.PlaceWindow(this);
            Top=0;
            Left=0;
            Width=screen.Width;
            Height=screen.Height;

            #if DEBUG
                this.Topmost = false;
            #else
                this.Topmost = true;
            #endif
            _Left.Width = new GridLength(_Position.X*screen.Scale);
            _Width.Width = new GridLength(_ObjWidth);
            _Top.Height  = new GridLength(_Position.Y*screen.Scale);
            _Height.Height = new GridLength(_ObjHeight);
            _Position.X += obj.ActualWidth;
        }

        //--- Property ShowRolls ---------------------------------------
        public bool ShowRolls
        {
            set { RollPap.Visibility = RollAbs.Visibility = Visibility.Visible;}
        }
        
        //--- Pad_SizeChanged ------------------------------
        private void Pad_SizeChanged(object sender, SizeChangedEventArgs e)
        {            
            RxScreen screen =new RxScreen();
            double left = _Position.X*screen.Scale;
            double top  = _Position.Y*screen.Scale;
            double border=2;
            if (screen.Surface || screen.Elo)
            {
                top  += _ObjHeight;
                left -= 32;
                border=10;
            }
            if (top  + Pad.ActualHeight > Height-border)
            {
               left += _ObjWidth;
               top  = Height-Pad.ActualHeight-border;
            }
            if (left + Pad.ActualWidth  > Width-border )   left = Width-Pad.ActualWidth-border;
            Canvas.SetLeft (Pad, left);        
            Canvas.SetTop  (Pad, top);
        }

        //--- Property result ------------------------------------------------
        private string _result="0";
        public string Result
        {
            get { return _result; }
            private set { _result = value; this.OnPropertyChanged("Result"); }
        }

        //--- RxNumPad_KeyDown ----------------------------------------------
        void RxNumPad_KeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            if (e.Key==Key.Decimal || e.Key==Key.OemPeriod)     _handle_key(".");
            else if (e.Key==Key.Return || e.Key==Key.Back || e.Key==Key.Escape) _handle_key(e.Key.ToString());
            else 
            {
                int code=System.Windows.Input.KeyInterop.VirtualKeyFromKey(e.Key);         
                if (e.Key>=Key.NumPad0 && e.Key<=Key.NumPad9) code = '0'+e.Key-Key.NumPad0;
                _handle_key(new string(Convert.ToChar(code),1));
            } 
        }

        //--- button clicked -----------------------------
        static int _Time=Environment.TickCount;
        private void button_clicked(object sender, RoutedEventArgs e)
        {
            //--- filter out double clicks ---
            {
                if  (Environment.TickCount-_Time<50) return;
                _Time=Environment.TickCount;
            }
            Button button = sender as Button;
            if (button==null) return;
            if (button.CommandParameter==null)  _handle_key(button.Content.ToString());
            else                                _handle_key(button.CommandParameter.ToString());
        }    

        //--- _handle_key --------------------------------------------------
        private void _handle_key(string key)
        {
            if (key.Equals("Escape"))       this.DialogResult = false;
            else if (key.Equals("Return"))  
                this.DialogResult = true;
            else if (key.Equals("+="))      
            {          
                double val=_OrgValue+Rx.StrToDouble(Result);
                Result = val.ToString(CultureInfo.InvariantCulture);     
                this.DialogResult = true;
            }
            else if (key.Equals("-="))      
            {
                double val=_OrgValue-Rx.StrToDouble(Result);
                Result = val.ToString(CultureInfo.InvariantCulture);     
                this.DialogResult = true;
            }
            else if (key.Equals("PAP"))
            {
                double val=Rx.StrToDouble(Result);
                Result = (val * 550).ToString(CultureInfo.InvariantCulture);    // 550 m per roll     
                this.DialogResult = true;
            }
            else if (key.Equals("ABS"))
            {
                double val=Rx.StrToDouble(Result);
                Result = (val * 420).ToString(CultureInfo.InvariantCulture);    // 420 m per roll     
                this.DialogResult = true;
            }
            else if (key.Equals("Back"))
            {
                if (Result.Length > 0) Result = Result.Remove(Result.Length - 1);
            }
            else if (key.Equals("+/-"))
            {
                if (Result[0]=='-') Result = Result.Remove(0,1);
                else                Result = '-'+Result;
            }
            else if (Result.Equals("0")) Result=key;
            else Result += key;
        }

        //--- property changd -----------------------------------
        public event PropertyChangedEventHandler PropertyChanged;
        private void OnPropertyChanged(String info)
        {
            if (PropertyChanged != null) PropertyChanged(this, new PropertyChangedEventArgs(info));
        }

        //--- special for the first touch ---------------------------------------------
        private void Window_TouchUp(object sender, System.Windows.Input.TouchEventArgs e)
        {
            button_clicked(e.Source, null);
        }
    }
}
