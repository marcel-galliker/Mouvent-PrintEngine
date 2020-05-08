using Infragistics.Windows.Controls;
using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.UserControls;
using RX_LabelComposer.External;
using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
// using System.Net.NetworkInformation;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Navigation;
using System.Windows.Threading;

// using Windows.UI.ApplicationSettings;  
// using Windows.UI.Popups;


namespace RX_DigiPrint.Views
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        //--- Constructor ------------------------------------------
        public MainWindow()
        {
            RxBindable.Dispatcher = Dispatcher;

            InitializeComponent();
            System.Diagnostics.PresentationTraceSources.DataBindingSource.Switch.Level = System.Diagnostics.SourceLevels.Critical;
            #if DEBUG__RIP
                {
                    RxScreen screen = new RxScreen();
                    if (!screen.Surface)
                    {                 
                        System.Diagnostics.PresentationTraceSources.DataBindingSource.Switch.Level = System.Diagnostics.SourceLevels.Critical;
                        Main.WindowStyle = WindowStyle.SingleBorderWindow;
                        Main.ResizeMode = ResizeMode.CanResize;
                    }
                }
            #endif

            this.Title=System.IO.Path.GetFileNameWithoutExtension(System.Windows.Forms.Application.ExecutablePath);
                        
     //       PrinterName.DataContext       = RxGlobals.PrinterProperties;

            Status.DataContext            = RxGlobals.RxInterface;
            PrinterStatus.DataContext     = RxGlobals.RxInterface;
            Counters.DataContext          = RxGlobals.PrinterStatus;
            TabLB701.DataContext          = RxGlobals.PrintSystem;
            TabDP803.DataContext          = RxGlobals.PrintSystem;
            TabLB702WB.DataContext        = RxGlobals.PrintSystem;
            TabLH702.DataContext          = RxGlobals.PrintSystem;
            MainNotConnected.DataContext  = RxGlobals.RxInterface;
            BlueTooth.DataContext         = RxGlobals.Bluetooth;
            BlueTooth.Visibility          = RxGlobals.Bluetooth.IsSupported ? Visibility.Visible : Visibility.Collapsed;
            User.DataContext              = RxGlobals.User;
            CleafOrder.DataContext        = RxGlobals.CleafOrder;
      
            RxGlobals.License.Update();          
            _LicenseChanged();

            License.OnLicenseChanged     += _LicenseChanged;
            RxGlobals.PrinterStatus.Image = PrinterStatus;
            RxGlobals.PrinterStatus.ErrorTypeChangedEvent += _ErrorTypeChanged;
            RxGlobals.PrinterStatus.LogType = ELogType.eErrStop;

            for(int i=0;i<TabCtrl.Items.Count; i++)
            {
                TabItemEx tab = TabCtrl.Items[i] as TabItemEx;
                if (tab.Name.Equals("TabEvents")) 
                    break;
                tab.Visibility = Visibility.Collapsed;
            }

            {
                FileVersionInfo info = FileVersionInfo.GetVersionInfo(Assembly.GetExecutingAssembly().Location);
                // DateTime date=File.GetLastWriteTime(Assembly.GetExecutingAssembly().Location);
                Version.Text = "V "+info.FileVersion;
            }

            RxGlobals.RxInterface.PropertyChanged += OnRxInterfacePropertyChanged;
            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
            RxGlobals.PrinterProperties.PropertyChanged += PrinterProperties_PropertyChanged;
            RxGlobals.Settings.PropertyChanged += Settings_PropertyChanged;
        }

        void PrinterProperties_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Host_Name"))
            {
                RxBindable.Invoke(()=>
                {
                    PrinterName.Text= RxGlobals.PrinterProperties.Host_Name;
                    PrinterName.InvalidateVisual();
                });
            }
        }

        //--- _LicenseChanged ---------------------------
        void _LicenseChanged()
        {
            User.ItemsSource = new EN_UserTypeList();
        }

        //--- Settings_PropertyChanged -----------------------------------------------
        void Settings_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Units")) _PrinterTypeChanged();
        }

        //--- Window_Loaded ------------------------------------------------------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            PrinterStatus.Online = RxGlobals.RxInterface.Connected;
        }

        //--- Window_Closed ------------------------------------------
        private void Window_Closed(object sender, EventArgs e)
        {
            RxGlobals.RxInterface.ShutDown();
            RxGlobals.Bluetooth.ShutDown();
            RxGlobals.Keyboard.ShutDown();
//            RxGlobals.UvLamp.ShutDown();
            //--- start explorer if not running already ---
            if (Process.GetProcessesByName("explorer").Length==0)
            {
                Process.Start(@"explorer.exe");
            }

            //--- kill all threads ----
            {
                string name=Path.GetFileName(System.Windows.Forms.Application.ExecutablePath);
                string cmd=string.Format("/IM {0} /F", name);
                Rx.StartProcess("taskkill.exe", cmd);
            }
        }

        //--- ErrorTypeChanged -------------------------------------
        private static Brush _InitialBrush;
        private void _ErrorTypeChanged(ELogType errorType)
        {
            if (_InitialBrush==null) _InitialBrush=TabEvents.Background;
            switch(errorType)
            {
                case ELogType.eErrWarn:  TabEvents.Background= Brushes.Yellow; break;
                case ELogType.eErrCont:  TabEvents.Background= Brushes.Red; break;
                case ELogType.eErrStop:  TabEvents.Background= Brushes.Red; break;
                case ELogType.eErrAbort: TabEvents.Background= Brushes.Red; break;
                default:                 TabEvents.Background= _InitialBrush; break;
            }
        }

        //--- Hyperlink_RequestNavigate -------------------------------------------
        private void Hyperlink_RequestNavigate(object sender, RequestNavigateEventArgs e)
        {
            Process.Start(new ProcessStartInfo(e.Uri.AbsoluteUri));
            e.Handled = true;
        }        
        
        //--- OnRxInterfacePropertyChanged ---------------------------------------------
        private void OnRxInterfacePropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Connected"))
            {
                PrinterStatus.Dispatcher.Invoke(()=>PrinterStatus.Online = RxGlobals.RxInterface.Connected);
            }
        }

        //--- PrintSystem_PropertyChanged --------------------------
        void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("PrinterType")) _PrinterTypeChanged();

            if (e.PropertyName.Equals("Changed"))   TabPrintSystem.Changed = RxGlobals.PrintSystem.Changed;
        }

        //--- _PrinterTypeChanged ----------------------------------------------------------------
        private void _PrinterTypeChanged()
        {
            bool tx = RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_TX801 || RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_TX802;
            bool lb = RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LB701 
                ||    RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LB702_UV
                ||    RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LB702_WB
                ||    RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LH702
                ||    RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_DP803;

            TabPrintQueue.Visibility = (RxGlobals.PrintSystem.PrinterType  == EPrinterType.printer_test_table 
                                        || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide
                                        || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide_only
                                        || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LH702
                                        ) ? Visibility.Collapsed : Visibility.Visible;
                                        
            Counters.Visibility      = (tx || lb) ? Visibility.Visible : Visibility.Collapsed;

            { //--- update the counters ------------------------
                CUnit unit = new CUnit("m");
                if (tx) CounterUnit1.Text= unit.Name+"/h";
                else    CounterUnit1.Text= unit.Name+"/min";

                CounterUnit2.Text=unit.Name;
                CounterUnit3.Text=unit.Name;

                Counters.DataContext = null;
                Counters.DataContext = RxGlobals.PrinterStatus;
            }

            switch (RxGlobals.PrintSystem.PrinterType)
            {
                case EPrinterType.printer_LB701:  
                    _activeView (TabLB701);
                    break;

                case EPrinterType.printer_LB702_UV:  
                    _activeView (TabLB702UV);
                    break;

                case EPrinterType.printer_LB702_WB:  
                    _activeView (TabLB702WB);
                    break;

                case EPrinterType.printer_LH702:  
                    _activeView (TabLH702);
                    break;

                case EPrinterType.printer_DP803:  
                    _activeView (TabDP803);
                    break;

                case EPrinterType.printer_TX801:
                case EPrinterType.printer_TX802:
                    _activeView (TabTex);
                    break;

                case EPrinterType.printer_test_slide:
                case EPrinterType.printer_test_slide_only:
                case EPrinterType.printer_test_table:
                    _activeView(TabTestTable);
                    break;

                case EPrinterType.printer_cleaf:
                    _activeView (TabCleaf);
                    break;
            }
            CleafOrder.Visibility = (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf)? Visibility.Visible : Visibility.Collapsed;
        }

        //--- _activeView -------------------------------
        private void _activeView(TabItemEx activeItem)
        {
            for (int i=1; i<TabCtrl.Items.Count; i++)
            {                
                TabItemEx item = TabCtrl.Items[i] as TabItemEx;
                if (item.Name.Equals("TabEvents")) break;
                if (item.Equals(activeItem) 
                || (activeItem.Equals(TabLH702) && item.Equals(TabLB702UV) && RxGlobals.PrintSystem.LH702_simulation)
                    )
                {
                    item.Visibility=Visibility.Visible;
  //                TabCtrl.SelectedIndex = i;
                }
                else item.Visibility=Visibility.Collapsed;
            }
        }

        //--- PrintQueueView_Loaded ---------------------------------------------------
        private void PrintQueueView_Loaded(object sender, RoutedEventArgs e)
        {

        }

        //--- Settings_Clicked -----------------------------------------------------
        private void Settings_Clicked(object sender, RoutedEventArgs e)
        {
            /*
            if (Settings.IsVisible) Settings.Visibility = Visibility.Hidden;
            else Settings.Visibility=Visibility.Visible;
            */
            Settings.Show();
        }

        //--- Bluetooth_Clicked -----------------------------------
        private void Bluetooth_Clicked(object sender, RoutedEventArgs e)
        {
            BluetoothWindow wnd = new BluetoothWindow();
            wnd.ShowDialog();
            RxGlobals.License.Update();
        }

        //--- User_PreviewMouseDown -------------------------------
        private void User_PreviewMouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            RxGlobals.License.Update();
        }

        //--- Counter_MouseDown -------------------------------------------
        private void Counter_MouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            if (RxGlobals.PrinterStatus.PrintState != EPrintState.ps_printing)
            {
                if (RxMessageBox.YesNo("Reset", "Reset counter", MessageBoxImage.Question, false))
                    RxGlobals.RxInterface.SendCommand(TcpIp.CMD_RESET_CTR);
            }
        }

    }
}
