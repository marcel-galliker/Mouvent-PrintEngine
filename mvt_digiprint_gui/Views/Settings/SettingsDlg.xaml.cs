using RX_DigiPrint.Models;
using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Windows;
using System.Windows.Media.Imaging;
using RX_Common;
using RX_DigiPrint.Models.Enums;
using System.Collections.ObjectModel;
using RX_DigiPrint.External;
using System.Threading;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.UserControls;
using RX_DigiPrint.Helpers;

namespace RX_DigiPrint.Views.Settings
{
    /// <summary>
    /// Interaction logic for SettingsDlg.xaml
    /// </summary>
    public partial class SettingsDlg : CustomWindow
    {
        private PrinterProperties _Settings;

        //--- Property WlanConnected ---------------------------------------
        private bool? _WlanConnected;
        public bool WlanConnected
        {
            get { return _WlanConnected != null && (bool)_WlanConnected; }
            set
            {
                if (_WlanConnected == null || (bool)_WlanConnected != value)
                {
                    _WlanConnected = value;
//                    WlanStatus.Kind = (bool)_WlanConnected ? MahApps.Metro.IconPacks.PackIconMaterialKind.Wifi : MahApps.Metro.IconPacks.PackIconMaterialKind.WifiOff;
                    WlanStatus.Source = (bool)_WlanConnected ? (BitmapImage)App.Current.Resources["tcp_online_ico"] : (BitmapImage)App.Current.Resources["tcp_offline_ico"];
                }
            }
        }

        //--- Wlan_Clicked ---------------------------------------------
        private void Wlan_Clicked(object sender, RoutedEventArgs e)
        {
            System.Windows.Point pt = WlanButton.PointToScreen(new System.Windows.Point(0, 0));
            RxGlobals.Popup = WlanPopup;
            WlanPopup.Open(WlanButton);
        }

        //--- WlanTimer ----------------------------------------------
        private void WlanTimer(int no)
        {
            ObservableCollection<RX_Wlan.WLAN_INTERFACE_INFO> interfaces = RX_Wlan.get_interfaces();
            WlanConnected = (interfaces != null) && interfaces.Count > 0 && interfaces[0].isState == RX_Wlan.WLAN_INTERFACE_STATE.wlan_interface_state_connected;
        }

        public SettingsDlg()
        {
            InitializeComponent();

            CB_Units.ItemsSource = new EN_UnitsList();

            Button_Save.IsEnabled = true;
            RxGlobals.Timer.TimerFct += WlanTimer;

            Init();
        }

        private void Confirm_Click(object sender, RoutedEventArgs e)
        {
            Save();
            DialogResult = true;
        }
        
        private void User_Click(object sender, RoutedEventArgs e)
        {
            UserAccount userAccount = new UserAccount();
            userAccount.ShowDialog();
        }
        

        //--- init ---------------------------------
        private void Init()
        {
            _Settings = RxGlobals.PrinterProperties.RxClone();
            DataContext = _Settings;
            
            FileVersionInfo info = FileVersionInfo.GetVersionInfo(Assembly.GetExecutingAssembly().Location);
            DateTime date = GetBuildTime();
            Version.Text = info.FileVersion;
            Build.Text = GetBuild();
            Date.Text = string.Format("{0} {1} {2}", date.Day, Rx.MonthName[date.Month], date.Year);
            LocalIpAddr.Text = RxGlobals.RxInterface.LocalAddress;

            if (_Settings.Host_DHCP) HOST_DHCP.IsChecked = true;
            else HOST_STATIC.IsChecked = true;
        }

        private string GetVersionAttribute(string key)
        {
            string result;
            if (GetAssemblyAttribute<AssemblyInformationalVersionAttribute>()
               .InformationalVersion.ToString().Split(new[] { ';' }, StringSplitOptions.RemoveEmptyEntries)
               .Select(part => part.Split('='))
               .ToDictionary(split => split[0], split => split[1]).TryGetValue(key, out result))
            {
                return result;
            }
            else
            {
                return string.Empty;
            }
        }

        private string GetBuild()
        {
            return GetVersionAttribute("Build");
        }


        private DateTime GetBuildTime()
        {

            string str = GetVersionAttribute("BuiltOn");
            string[] aStr = str.Split('.');
            if (3 == aStr.Length)
            {
                return new DateTime(Int32.Parse(aStr[2]), Int32.Parse(aStr[1]), Int32.Parse(aStr[0]));
            }

            return File.GetLastWriteTime(Assembly.GetExecutingAssembly().Location);

        }

        private T GetAssemblyAttribute<T>() where T : Attribute
        {
            Assembly ass = Assembly.GetExecutingAssembly();
            return (T)ass.GetCustomAttributes(typeof(T), false).FirstOrDefault();
        }

        //---  save -----------------------------------------
        private void Save()
        {
            // _Settings.Host_DHCP = (bool)HOST_DHCP.IsChecked;
            _Settings.SetNetworkSettings();
            RxGlobals.PrinterProperties = _Settings.RxClone();
            RxGlobals.PrinterProperties.Save();
        }        

        //--- About_Clicked ---------------------------------------------------
        private void About_Clicked(object sender, RoutedEventArgs e)
        {
            AboutDlg dlg = new AboutDlg(typeof(MainWindow).Assembly.GetName());
            dlg.ShowDialog();
        }

        //--- TeamViewer_Clicked ---------------------------------------------
        private void TeamViewer_Clicked(object sender, RoutedEventArgs e)
        {
            string ProgramsPath = System.Environment.GetFolderPath(System.Environment.SpecialFolder.ProgramFilesX86);
            RxGlobals.Events.AddItem(new LogItem("Starting TeamViewer"));
            Rx.StartProcess(ProgramsPath + @"\TeamViewer\TeamViewer.exe", null);
            if (Process.GetProcessesByName("rx_net_bridge").Length == 0)
            {
                Thread.Sleep(1000);
                Rx.StartProcess(ProgramsPath + @"\Mouvent\rx_net_bridge.exe", null);
                Thread.Sleep(1000);
                int cnt = Process.GetProcessesByName("rx_net_bridge").Length;
                RxGlobals.Events.AddItem(new LogItem("Starting rx_net_bridgt: cnt=" + cnt.ToString()));
            }
        }

        //--- WinSCP_Clicked ---------------------------------------------
        private void WinSCP_Clicked(object sender, RoutedEventArgs e)
        {
            string ProgramsPath = System.Environment.GetFolderPath(System.Environment.SpecialFolder.ProgramFilesX86);
            Rx.StartProcess(ProgramsPath + @"\WinSCP\WinSCP.exe", null);
        }

        //--- RestartMain_Clicked ---------------------------------------------
        private void RestartMain_Clicked(object sender, RoutedEventArgs e)
        {
            if (MvtMessageBox.YesNo("Restart", "Restarting the MAIN!", MessageBoxImage.Question, false))
            {
                RxGlobals.RxInterface.SendCommand(TcpIp.CMD_RESTART_MAIN);
            }
        }

        //--- RestartGUI_Clicked ---------------------------------------------
        private void RestartGUI_Clicked(object sender, RoutedEventArgs e)
        {
            if (MvtMessageBox.YesNo("Restart", "Restarting the GUI!", MessageBoxImage.Question, false))
            {
                Rx.StartProcess("shutdown", "/r /t 0");
            }
        }

        //--- ShutDown_Clicked ---------------------------------------------
        private void ShutDown_Clicked(object sender, RoutedEventArgs e)
        {
            if (MvtMessageBox.YesNo("Shut Down", "Shutting Down the GUI!", MessageBoxImage.Question, false))
            {
                Rx.StartProcess("shutdown", "/s /t 0");
            }
        }

        //--- Update_Clicked ---------------------------------------------
        private void Update_Clicked(object sender, RoutedEventArgs e)
        {
            if (MvtMessageBox.YesNo("Windows Update", "Updating Windows!", MessageBoxImage.Question, false))
            {
                Rx.StartProcess("C:\\Windows\\System32\\control.exe", "/name Microsoft.WindowsUpdate");
            }
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            RxGlobals.Screen.PlaceWindow(this);
        }
    }
}
