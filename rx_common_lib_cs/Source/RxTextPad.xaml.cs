using System;
using System.Windows;
using System.Windows.Controls;
using System.ComponentModel;
using System.Windows.Media;
using System.Windows.Input;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using System.Diagnostics;

namespace RX_Common
{
    public partial class RxTextPad : Window, INotifyPropertyChanged
    {       
        //--- For Keyboard Window --------------
        delegate void WinEventDelegate(IntPtr hWinEventHook, uint eventType, IntPtr hwnd, int idObject, int idChild, uint dwEventThread, uint dwmsEventTime);

        const uint WINEVENT_OUTOFCONTEXT = 0;
        const uint EVENT_SYSTEM_FOREGROUND = 3;

        [DllImport("user32.dll")]
        static extern bool UnhookWinEvent(IntPtr hWinEventHook);

        [DllImport("user32.dll")]
        static extern IntPtr SetWinEventHook(uint eventMin, uint eventMax, IntPtr hmodWinEventProc, WinEventDelegate lpfnWinEventProc, uint idProcess, uint idThread, uint dwFlags);

        [DllImport("user32.dll")]
        static extern int GetWindowText(IntPtr hWnd, StringBuilder text, int count);

        [DllImport("user32.dll")]
        private static extern IntPtr GetForegroundWindow();

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool SetForegroundWindow(IntPtr hWnd);

        IntPtr                      m_hhook;
        private WinEventDelegate    _winEventProc;
        static IntPtr               _puttyWnd;

        //-------------

        private int     _shift=0;
        private bool    _shuttingDown=false;
        private TextBox _Target;
        private Point   _Position;
        private double  _ObjWidth=0;
        private double  _ObjHeight=0;
        RxScreen        _Screen =new RxScreen();

        static private List<List<List<string>>> _Keys = new List<List<List<string>>>()
        {
            //--- standard ----
            new List<List<string>>
            {
                new List<string>{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0","Back"},
                new List<string>{"q", "w", "e", "r", "t", "z", "u", "i", "o", "p","ENTER"},
                new List<string>{"a", "s", "d", "f", "g", "h", "j", "k", "l", "/"},
                new List<string>{".", "y", "x", "c", "v", "b", "n", "m", "-", "_", "\\"}
            },
            //--- caps ---------
            new List<List<string>>
            {
                new List<string>{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0","Back"},
                new List<string>{"Q", "W", "E", "R", "T", "Z", "U", "I", "O", "P","ENTER"},
                new List<string>{"A", "S", "D", "F", "G", "H", "J", "K", "L", "/"},
                new List<string>{".", "Y", "X", "C", "V", "B", "N", "M", "-", "_", "\\"}
            },
            //--- symbols ------------------
            new List<List<string>>
            {
                new List<string>{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0","Back"},
                new List<string>{"@", "#", "°", "$", "€", "+", "-", "?", "&", "%","ENTER"},
                new List<string>{"ä", "ö", "ü", "é", "è", "à", "â", "", "", "/"},
                new List<string>{".", ",", ":", ";", "", "", "", "", "", "_", "\\"}
            }
        };
      
        //--- creator -----------------------------------
        public RxTextPad(TextBox obj)
        {
            InitializeComponent();

            this.DataContext = this;
            PreviewKeyDown += RxTextPad_KeyDown;
//            KeyDown += RxTextPad_KeyDown;

            _Target = obj;
            if (obj!=null)
            {
                _Position  = obj.PointToScreen(new Point(0, 0));
                _ObjWidth  = obj.ActualWidth;
                _ObjHeight = obj.ActualHeight;
            }

            RxScreen screen =new RxScreen();
            screen.PlaceWindow(this);

            Pad.Visibility = Visibility.Hidden;

            for (int i=0; i<_Keys[0].Count; i++)
            {
                _CreateKeys(1+i, _Keys[0][i]);
            }

            if (obj!=null) 
            {
                Result = obj.Text;
            }
            else
            {
                AllowsTransparency = false;
                WindowStyle        = WindowStyle.SingleBorderWindow;
                ResizeMode         = ResizeMode.CanMinimize;
                Background         = Brushes.White;
                TopRow.Visibility  = Visibility.Collapsed;
                _winEventProc      = new WinEventDelegate(WinEventProc);
                m_hhook            = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, IntPtr.Zero, _winEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
            }

            #if (!DEBUG)
                this.Topmost = true;
            #endif
        }

        //--- destructor -------------------------------
        ~RxTextPad()
        {
            UnhookWinEvent(m_hhook);
        }

        //--- ShutDown ------------------------------------
        public void ShutDown()
        {
            _shuttingDown = true;
            Close();
        }

        //--- Window_Closing -------------------------------------
        private void Window_Closing(object sender, CancelEventArgs e)
        {
            if (_Target==null && !_shuttingDown)
            {
                Visibility = Visibility.Collapsed;
                e.Cancel = true;
            }
        }

        //--- Window_IsVisibleChanged ----------------------------
        private void Window_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue && IsLoaded)
            {
                SetForegroundWindow(_puttyWnd);
                Visibility  = Visibility.Visible;
                #if (!DEBUG) 
                    Topmost     = true;
                #endif
            }
        }

        //--- WinEventProc -------------------------
        void WinEventProc(IntPtr hWinEventHook, uint eventType, IntPtr hwnd, int idObject, int idChild, uint dwEventThread, uint dwmsEventTime)
        {
            if (eventType == EVENT_SYSTEM_FOREGROUND)
            {
                StringBuilder Buff = new StringBuilder(500);
                GetWindowText(hwnd, Buff, Buff.Capacity);

                if (Buff.Length>0) Console.WriteLine("ForegroundWindow=>>{0}<<", Buff);
                string name=Buff.ToString();
                if (name.Contains("PuTTY")) 
                    _puttyWnd = hwnd;
            }
        }

        //--- Window_SizeChanged -------------------------------------
        private void Window_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (_Target==null) 
                SizeToContent   = SizeToContent.WidthAndHeight;
        }

        //--- Pad_SizeChanged ------------------------------
        private void Pad_SizeChanged(object sender, SizeChangedEventArgs e)
        {            
            RxScreen screen =new RxScreen();
            double left = _Position.X*screen.Scale;
            double top  = _Position.Y*screen.Scale;
            double border=2;
            if (screen.Surface)
            {
                top  += _ObjHeight;
                left -= 32;
                border=10;
            }
            if (top  + Pad.ActualHeight > screen.Height*screen.Scale-border)
            {
               left += _ObjWidth;
               top  = screen.Height*screen.Scale-Pad.ActualHeight-border;
            }
            if (left + Pad.ActualWidth  > screen.Width*screen.Scale-border )   left = screen.Width*screen.Scale-Pad.ActualWidth-border;
            Canvas.SetLeft (Pad, left); 
            Canvas.SetTop  (Pad, top);
        }

        //--- Window_Loaded -------------------------------------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {            
            if (_Target==null) // Keyboard Window
            {
                Rx.wnd_remove_close_button(this);
                MainGrid.Width  = Pad.ActualWidth;
                MainGrid.Height = Pad.ActualHeight;
                Back.Visibility = Visibility.Collapsed;
                Pad.Visibility  = Visibility.Visible;
                SizeToContent   = SizeToContent.WidthAndHeight;
                Width           = Pad.ActualWidth;
                Height          = Pad.ActualHeight;
                Top             = _Screen.Height*_Screen.Scale-this.ActualHeight;
                Left            = _Screen.Width*_Screen.Scale -this.ActualWidth;
                #if (!DEBUG) 
                    Topmost     = true;
                #endif
                return; 
            }
            //--- place panel -----------------
            Point pt = new Point(0,0);
            FrameworkElement obj=_Target as FrameworkElement;
            while(true)
            {
                try
                {
                    pt = obj.PointToScreen(new Point(0, 0));
                    break;
                }
                catch(Exception){obj =(FrameworkElement) LogicalTreeHelper.GetParent(obj);}
            }
            #if (!DEBUG) 
                this.Topmost=true;
            #endif
            
            double width=(_Screen.Width-pt.X)*_Screen.Scale-_Target.ActualWidth*_Screen.FactX;
            double height=(_Screen.Height-pt.Y)*_Screen.Scale-_Target.ActualHeight*_Screen.FactY;

            Back.BorderThickness = new Thickness(pt.X*_Screen.Scale, pt.Y*_Screen.Scale, width, height);
            Pad.Visibility = Visibility.Visible;
        }

        //--- _CreateKeys --------------------------------------------------
        private void _CreateKeys(int row, List<string> keys)
        {
            int pos;
            StackPanel stack = Pad.Children[row] as StackPanel;

            for (pos=0; pos<keys.Count; pos++)
            {
                Button b = new Button();
                if (keys[pos]==" ") HorizontalAlignment=HorizontalAlignment.Stretch;
                if (keys[pos].Length>1) b.Background = ClearButton.Background;
                b.Content = keys[pos].ToString();
                b.Click += button_Click;
                stack.Children.Add(b);
            }
        }

        //--- Property result ------------------------------------------------
        private string _result;
        public string Result
        {
            get { return _result; }
            private set { _result = value; this.OnPropertyChanged("Result"); }
        }

        //--- RxTextPad_KeyDown ----------------------------------------------
        static System.Windows.Input.KeyConverter converter = new System.Windows.Input.KeyConverter();

        void RxTextPad_KeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            Debug.Print("key=>>{0}<<", e.Key);
            _handle_key(converter.ConvertToString(e.Key), Rx.GetCharFromKey(e.Key));
        }

        //--- button clicked -----------------------------
        static int _Time=Environment.TickCount;
        private void button_Click(object sender, RoutedEventArgs e)
        {
            //--- filter out double clicks ---
            {
                if  (Environment.TickCount-_Time<50) return;
                _Time=Environment.TickCount;
            }

            Button button = sender as Button;
            if (button==null) return;

            _handle_key(button.Content.ToString(), (char)0);
        }    

        //--- close_Clicked ----------------------
        private void close_Clicked(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        //--- _send ---------------------
        private void _send(string str)
        {
            IntPtr wnd = GetForegroundWindow();
            try
            {
                SetForegroundWindow(_puttyWnd);
                System.Windows.Forms.SendKeys.SendWait(str);    
            }
            catch(Exception)
            { };
            SetForegroundWindow(wnd);
        }

        //--- _handle_key --------------------------------------------------
        private void _handle_key(string key, char ch)
        {            
            if (key.Equals("ENTER"))  
            {
                if (_Target==null)  _send("\r");
                else                this.DialogResult = true;            
            }
            else if (key.Equals("System"))
            {
                if (_Target!=null)  this.DialogResult = true;
            }
            else if (key.Contains("Back"))
            {
                if (_Target==null) _send("\b");
                else if (Result.Length > 0) Result = Result.Remove(Result.Length - 1);
            }
            else if  (ch!=(char)0)
            {
                if (_Target==null) _send(ch.ToString());
                else               Result  += ch.ToString();
            }
            else if (key.Length==1) 
            {
                if (_Target==null) _send(key);
                else Result += key;
            }
            else if (key.Equals("Esc"))     
            {
                if (_Target==null) _send(((char)27).ToString());
                else
                {
                    this.DialogResult = false;
                }        
            }
            else if (key.Equals("Clear"))
            {
               Result="";
            }
            else if (key.Contains("Shift"))
            {
                int row=0;
                _shift = (_shift+1) % _Keys.Count;
                foreach(var item  in Pad.Children)
                {
                    StackPanel stk = item as StackPanel;
                    if (stk!=null)
                    {
                        for(int i=0; i<stk.Children.Count; i++)
                        {
                            Button b = stk.Children[i] as Button;
                            if (b!=null) b.Content = _Keys[_shift][row][i];
                        }
                        row++;
                    }
                }
            }
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
            button_Click(e.Source, e);
        }
    }
}
