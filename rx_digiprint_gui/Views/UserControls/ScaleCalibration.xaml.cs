using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for ScaleCalibration.xaml
    /// </summary>
    public partial class ScaleCalibration : Window
    {
        private int     _No;
        private Brush   _ColorReady = (Brush)App.Current.Resources["RX.Green"];
        private Brush   _ColorError = Brushes.Red;
        private static BitmapImage  _img_ok   = new BitmapImage(new Uri("../../Resources/Bitmaps/confirm.ico", UriKind.Relative));


        private Point   _Position;
        private double  _ObjWidth=0;
        private double  _ObjHeight=0;

        private Rectangle[]         _Sensor     = new Rectangle[3];
        private Image[]             _Checked    = new Image[3];

        public ScaleCalibration(Control obj, Point pos, int no)
        {
            // DataContext : RxProgress
            InitializeComponent();

            _No = no;
            _Position  = obj.PointToScreen(new Point(0, 0));
            _ObjWidth  = obj.ActualWidth;
            _ObjHeight = obj.ActualHeight;
            
            RxScreen screen = RxScreen.Screen;
            screen.PlaceWindow(this);

            if (screen.Surface) Back.BorderThickness = new Thickness(_Position.X*screen.Scale, _Position.Y*screen.Scale, (screen.Width-_Position.X)*screen.Scale-obj.ActualWidth*screen.FactX, (screen.Height-_Position.Y)*screen.Scale-obj.ActualHeight*screen.FactX-1);
            else                Back.BorderThickness = new Thickness(_Position.X,              _Position.Y,               screen.Width-_Position.X-obj.ActualWidth,                             screen.Height-_Position.Y-obj.ActualHeight-1);

           #if (!DEBUG)
                this.Topmost = true;
           #endif
            
            _Position = pos;
            Weight.Text = (Properties.Settings.Default.Scales_Calibration_Weight/1000.0).ToString();
            RxGlobals.Timer.TimerFct += _timer_Tick;
        }

        //--- Window_Closed ----------------------------------------------
        private void Window_Closed(object sender, EventArgs e)
        {
            RxGlobals.Timer.TimerFct -= _timer_Tick;
        }

        //--- GroupBox_SizeChanged ------------------------------------
        private void GroupBox_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            RxScreen screen = RxScreen.Screen;
            if (_Position.X>500)  GroupBox.Margin = new Thickness(screen.Width*screen.Scale-GroupBox.Width, _Position.Y*screen.Scale, 0, 0);
            else                  GroupBox.Margin = new Thickness(_Position.X*screen.Scale, (screen.Height*screen.Scale-GroupBox.ActualHeight)/2, 0, 0);
        }

        //--- Property InkName ---------------------------------------
        public string InkName
        {
            set {   GroupBox.Header = "Calibrate "+value; }
        }   
            
        //--- _timer_Tick ---------------------------------------------------
        private void _timer_Tick(int tickNo)
        {    
            int i;
            bool flash = (tickNo&1)==0;

            TcpIp.SScaleStat item = RxGlobals.Scales[_No];

            switch(item.state)
            {
                case 0:                     Zero.Fill = Brushes.Transparent;                break;
                case TcpIp.SCL_CAL_ZERO:    Zero.Fill = flash? _ColorReady:Brushes.White;   break;
                default:                    Zero.Fill = _ColorReady;                        break;
            }
            for (i=0; i<_Sensor.Length; i++)
            {
                if (item.sensorState==null) {_Sensor[i].Fill = Brushes.Transparent; _Checked[i].Visibility=Visibility.Collapsed;}
                else
                {
                    switch(item.sensorState[i])
                    {
                        case TcpIp.SCL_SENS_OK:     _Sensor[i].Fill = _ColorReady;                            _Checked[i].Visibility=Visibility.Visible;   break;
                        case TcpIp.SCL_SENS_BUSY:   _Sensor[i].Fill = flash? _ColorReady:Brushes.White;       _Checked[i].Visibility=Visibility.Collapsed; break;
                        case TcpIp.SCL_SENS_ERROR:  _Sensor[i].Fill = flash? _ColorError:Brushes.Transparent; _Checked[i].Visibility=Visibility.Collapsed; break;
                        case TcpIp.SCL_SENS_OK_ERR: _Sensor[i].Fill = flash? _ColorError:Brushes.Transparent; _Checked[i].Visibility=Visibility.Visible;   break;
                        default:                    _Sensor[i].Fill = Brushes.Transparent;                    _Checked[i].Visibility=Visibility.Collapsed; break;
                    }
                }
            }

            Save.IsChecked  = (item.state==TcpIp.SCL_CAL_OK);
            int weight = (int)(1000.0*Rx.StrToDouble(Weight.Text));
            Start.IsEnabled = weight>=2000; 
        }

        //--- _init_sensor ----------------------------
        private Rectangle _init_sensor(Rectangle rect)
        {
            rect.Width    = rect.Height = 30;
            rect.RadiusX  = rect.RadiusY = rect.Width/2;
            rect.Margin   = new Thickness(rect.Width/3);
            rect.StrokeThickness = 2;
            rect.Stroke   = Brushes.Black;
            rect.Visibility = Visibility.Visible;
            return rect;
        }

        //--- ScalePlateLoaded --------------------------------------------
        private void ScalePlateLoaded(object sender, RoutedEventArgs e)
        {
            int i;
            
            _init_sensor(Zero);

            for (i=0; i<_Sensor.Length; i++) 
            {
                _Sensor[i]  = _init_sensor(new Rectangle());
                _Checked[i] = new Image(){Source=_img_ok, Width=_Sensor[i].Width, Margin=_Sensor[i].Margin, Visibility=Visibility.Collapsed};

            }
            _Sensor[0].HorizontalAlignment = _Checked[0].HorizontalAlignment = HorizontalAlignment.Right;
            _Sensor[0].VerticalAlignment   = _Checked[0].VerticalAlignment   = VerticalAlignment.Bottom;
            _Sensor[1].HorizontalAlignment = _Checked[1].HorizontalAlignment = HorizontalAlignment.Left;
            _Sensor[1].VerticalAlignment   = _Checked[1].VerticalAlignment   = VerticalAlignment.Bottom;
            _Sensor[2].HorizontalAlignment = _Checked[2].HorizontalAlignment = HorizontalAlignment.Center;
            _Sensor[2].VerticalAlignment   = _Checked[2].VerticalAlignment   = VerticalAlignment.Top;

            for (i=0; i<_Sensor.Length; i++) 
            {
                ScalePlate.Children.Add(_Sensor[i]);            
                ScalePlate.Children.Add(_Checked[i]);            
            }
        }

        //--- Start_Clicked ---------------------------------------------
        private void Start_Clicked(object sender, RoutedEventArgs e)
        {
            TcpIp.SScalesCalibrateCmd msg = new TcpIp.SScalesCalibrateCmd();
            msg.no      = _No;
            msg.weight  = (int)(1000.0*Rx.StrToDouble(Weight.Text));

            Properties.Settings.Default.Scales_Calibration_Weight = msg.weight;
            Properties.Settings.Default.Save();

            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SCALES_CALIBRATE, ref msg);
        }

        //--- Save_Clicked ---------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            TcpIp.SScalesCalibrateCmd msg = new TcpIp.SScalesCalibrateCmd(){no=_No};
            if (Save.IsChecked) RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SCALES_SAVE_CFG, ref msg);
            else                RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SCALES_LOAD_CFG, ref msg);
//            Visibility = Visibility.Collapsed;
            Close();
        }

        //--- Reload_Clicked ---------------------------------------------
        private void Reload_Clicked(object sender, RoutedEventArgs e)
        {
            TcpIp.SScalesCalibrateCmd msg = new TcpIp.SScalesCalibrateCmd(){no=_No};
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SCALES_LOAD_CFG, ref msg);            
//            Visibility = Visibility.Collapsed;           
            Close();
        }
    }
}
