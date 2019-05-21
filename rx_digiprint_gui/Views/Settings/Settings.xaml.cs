using RX_Common;
using RX_DigiPrint.External;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;

namespace RX_DigiPrint.Views.Settings
{
    /// <summary>
    /// Interaction logic for Settings.xaml
    /// </summary>
    public partial class Settings : UserControl
    {
        //--- Animations ---------------------
        private static double time=300;
        private Storyboard                      _sb         = new Storyboard();
        private Duration                        _duration   = new Duration(TimeSpan.FromMilliseconds(time));
        private DoubleAnimation                 _animation1 = new DoubleAnimation();
        private Timer _Timer;
        private bool _isOpen = false;
        private PrinterProperties       _Settings;    
                           
        /*
        private ObjectAnimationUsingKeyFrames   _animation2 = new ObjectAnimationUsingKeyFrames();
        private DiscreteObjectKeyFrame          _key2       = new DiscreteObjectKeyFrame();
        private ObjectAnimationUsingKeyFrames   _animation3 = new ObjectAnimationUsingKeyFrames();
        private DiscreteObjectKeyFrame          _key3       = new DiscreteObjectKeyFrame();
        */

        //---  Constructor -------------------------------
        public Settings()
        {
            InitializeComponent();
                    
            _Settings = RxGlobals.PrinterProperties.RxClone();    
            DataContext = _Settings;

            _sb.Duration        = _duration;
            Resources.Add("unique_id", _sb);

            //--- animation1: this.Width -----------
            bind_animation(_animation1, Border, "Width");
            _animation1.Duration = _duration;
            _sb.Children.Add(_animation1);

            /*
            //--- animatio2: this.Background ----------
            bind_animation(_animation2, this, "Background");
            _animation2.BeginTime = TimeSpan.FromMilliseconds(time);
            _key2.KeyTime = TimeSpan.FromMilliseconds(0);
            _animation2.KeyFrames.Add(_key2);
            _sb.Children.Add(_animation2);

            //--- animation2: SettingsGrid.Visibility -----------
            bind_animation(_animation3, SettingsGrid, "Visibility");
            _animation3.BeginTime = TimeSpan.FromMilliseconds(time);
            _key3.KeyTime = TimeSpan.FromMilliseconds(0);
            _animation3.KeyFrames.Add(_key3);
            _sb.Children.Add(_animation3);
            */
//            init();
            close(true);
            
            _Timer = new Timer(OnTimer, this, -1, 0);
        }

        //--- bind_animation --------------------------------------------------
        private void bind_animation(DependencyObject animation, DependencyObject obj,  string property)
        {
            Storyboard.SetTarget(animation, obj);
            Storyboard.SetTargetProperty(animation, new PropertyPath(property));
        }
        
        //--- Show -----------------------------------------
        public void Show()
        {
            if (_isOpen)
            {
                _Timer.Change(-1, 0);
                close(false);
            }
            else open();
        }

        //--- init ---------------------------------
        private void init()
        {
            _Settings = RxGlobals.PrinterProperties.RxClone();    
            DataContext = _Settings;

            FileVersionInfo info = FileVersionInfo.GetVersionInfo(Assembly.GetExecutingAssembly().Location);
            DateTime date=File.GetLastWriteTime(Assembly.GetExecutingAssembly().Location);
            Version.Text = info.FileVersion;
            Date.Text    = string.Format("{0} {1} {2}", date.Day, Rx.MonthName[date.Month], date.Year);
            LocalIpAddr.Text = RxGlobals.RxInterface.LocalAddress;

            if (_Settings.Host_DHCP) HOST_DHCP.IsChecked=true;
            else                     HOST_STATIC.IsChecked=true;
        }

        //---  save -----------------------------------------
        private void save()
        {
            _Settings.Host_DHCP = (bool)HOST_DHCP.IsChecked;
            _Settings. SetNetworkSettings();
            RxGlobals.PrinterProperties = _Settings.RxClone();
            RxGlobals.PrinterProperties.Save();
        }

        //--- open -----------------------------------------------
        public void open()
        {                
            init();
            _animation1.To = MaxWidth;           
            /*
            _animation2.BeginTime = _animation3.BeginTime = TimeSpan.FromMilliseconds(0);
            _key2.Value=(Brush)FindResource("HeaderCellItemNormalBackgroundBrush");
            _key3.Value=Visibility.Visible;
            */
            _sb.Begin();
            _isOpen = true;
            Button_Save.IsEnabled = true;
            RxGlobals.Timer.TimerFct += WlanTimer;
        }

        //--- Wlan_Clicked ---------------------------------------------
        private void Wlan_Clicked(object sender, RoutedEventArgs e)
        {
            System.Windows.Point pt = WlanButton.PointToScreen(new System.Windows.Point(0,0)); 
            RxGlobals.Popup = WlanPopup;
            WlanPopup.Open(WlanButton);
        }   

        //--- Property WlanConnected ---------------------------------------
        private bool? _WlanConnected;
        public bool WlanConnected
        {
            get { return _WlanConnected!=null && (bool)_WlanConnected; }
            set {   if (_WlanConnected==null || (bool)_WlanConnected != value)
                    {
                        _WlanConnected = value; 

                        WlanStatus.Source = (bool)_WlanConnected ? (BitmapImage)App.Current.Resources["tcp_online_ico"]:(BitmapImage)App.Current.Resources["tcp_offline_ico"];
                    }
                }
        }
        
        //--- WlanTimer ----------------------------------------------
        private void WlanTimer(int no)
        {
            ObservableCollection<RX_Wlan.WLAN_INTERFACE_INFO> interfaces = RX_Wlan.get_interfaces(); 
            WlanConnected = (interfaces!=null) &&  interfaces.Count>0 && interfaces[0].isState==RX_Wlan.WLAN_INTERFACE_STATE.wlan_interface_state_connected;
        }

        //--- close -------------------------------------------------
        private void close(bool init)
        {              
            RxGlobals.Timer.TimerFct -= WlanTimer;     
            if (init) 
            {
                // Background=Brushes.Transparent;
                // SettingsGrid.Visibility=Visibility.Hidden;
                Border.Width=0;
            }
            else
            {
                _animation1.To = 0;
                /*
                _animation2.BeginTime = _animation3.BeginTime = TimeSpan.FromMilliseconds(time);
                _key2.Value=Brushes.Transparent;
                _key3.Value=Visibility.Hidden;
                */
                _sb.Begin();
            }
            _isOpen = false;
            Button_Save.IsEnabled = false;
        }
        
        private static void OnTimer(object state)
        {
            Settings self = state as Settings;

            if (self!=null)
            {
                self.Dispatcher.Invoke(self.open);
            }
        } 

        //--- Save_Clicked ------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            save();
            close(false);
        }

        //--- Cancel_Clicked -------------------------------------------
        private void Cancel_Clicked(object sender, RoutedEventArgs e)
        {
            init();
            close(false);
        }

        //--- Calibtare_Clicked ---------------------------------------------
        private void Calibtare_Clicked(object sender, RoutedEventArgs e)
        {
            string  ProgramsPath=System.Environment.GetFolderPath(System.Environment.SpecialFolder.ProgramFiles);
            Rx.StartProcess(ProgramsPath+@"\Elo Touch Solutions\EloConfig.exe", "/align");
        }

        //--- TeamViewer_Clicked ---------------------------------------------
        private void TeamViewer_Clicked(object sender, RoutedEventArgs e)
        {
            string  ProgramsPath=System.Environment.GetFolderPath(System.Environment.SpecialFolder.ProgramFilesX86);
            RxGlobals.Events.AddItem(new LogItem("Starting TeamViewer"));
            Rx.StartProcess(ProgramsPath+@"\TeamViewer\TeamViewer.exe", null);
            if (Process.GetProcessesByName("rx_net_bridge").Length==0)
            {                
                Thread.Sleep(1000);
                Rx.StartProcess(ProgramsPath+@"\Mouvent\rx_net_bridge.exe", null);
                Thread.Sleep(1000);
                int cnt=Process.GetProcessesByName("rx_net_bridge").Length;                
                RxGlobals.Events.AddItem(new LogItem("Starting rx_net_bridgt: cnt="+cnt.ToString()));
            }
        }

        //--- WinSCP_Clicked ---------------------------------------------
        private void WinSCP_Clicked(object sender, RoutedEventArgs e)
        {
            string  ProgramsPath=System.Environment.GetFolderPath(System.Environment.SpecialFolder.ProgramFilesX86);
            Rx.StartProcess(ProgramsPath+@"\WinSCP\WinSCP.exe", null);
        }

        //--- RestartMain_Clicked ---------------------------------------------
        private void RestartMain_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxMessageBox.YesNo("Restart", "Restarting the MAIN!",  MessageBoxImage.Question, false))
            { 
                 RxGlobals.RxInterface.SendCommand(TcpIp.CMD_RESTART_MAIN);
            }
        }

        //--- RestartGUI_Clicked ---------------------------------------------
        private void RestartGUI_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxMessageBox.YesNo("Restart", "Restarting the GUI!",  MessageBoxImage.Question, false))
            { 
                Rx.StartProcess("shutdown", "/r /t 0");
            }
        }

        //--- ShutDown_Clicked ---------------------------------------------
        private void ShutDown_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxMessageBox.YesNo("Shut Down", "Shutting Down the machine!",  MessageBoxImage.Question, false))
            { 
                Rx.StartProcess("shutdown", "/s /t 0");
            }
        }

        //--- Update_Clicked ---------------------------------------------
        private void Update_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxMessageBox.YesNo("Windows Update", "Updating Windows!",  MessageBoxImage.Question, false))
            { 
                Rx.StartProcess("C:\\Windows\\System32\\control.exe", "/name Microsoft.WindowsUpdate");
            }
        }

        //--- About_Clicked ---------------------------------------------------
        private void About_Clicked(object sender, RoutedEventArgs e)
        {
            AboutDlg dlg = new AboutDlg(typeof(MainWindow).Assembly.GetName());
            dlg.ShowDialog();
        }
   }
}
