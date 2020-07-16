using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.Settings;
using RX_DigiPrint.Views.UserControls;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
// using System.Net.NetworkInformation;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Navigation;

// using Windows.UI.ApplicationSettings;
// using Windows.UI.Popups;


namespace RX_DigiPrint.Views
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        private List<TabItem> PrinterTypeTabs;

        public static event EventHandler PrinterTypeChangedEventHandler;

        public event PropertyChangedEventHandler PropertyChanged;

        private string _SelectedTabName;
        public string SelectedTabName
        {
            get { return _SelectedTabName; }
            set { _SelectedTabName = value; OnPropertyChanged("SelectedTabName"); }
        }
       
        private Brush _TabHeaderUnderlineColor;
        public Brush TabHeaderUnderlineColor
        {
            get { return _TabHeaderUnderlineColor; }
            set { _TabHeaderUnderlineColor = value; OnPropertyChanged("TabHeaderUnderlineColor"); }
        }
        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }


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
                        
            TabLB701.DataContext          = RxGlobals.PrintSystem;
            TabDP803.DataContext          = RxGlobals.PrintSystem;
            TabLB702WB.DataContext        = RxGlobals.PrintSystem;
            TabLH702.DataContext          = RxGlobals.PrintSystem;
            MainNotConnected.DataContext  = RxGlobals.RxInterface;
            TabHeaderUnderline.DataContext = this;
            TabCtrl.DataContext = this;

            RxGlobals.License.Update();
            
            _InitTabs();

            RxGlobals.PrinterStatus.ErrorTypeChangedEvent += _ErrorTypeChanged;
            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
            
            RxGlobals.Settings.PropertyChanged += Settings_PropertyChanged;

            MouventCommandButtons.SettingsClicked += new EventHandler(CommandButtonsSettingsClicked);

            RxGlobals.User.PropertyChanged += User_PropertyChanged;
        }

        
        void CommandButtonsSettingsClicked(object sender, EventArgs e)
        {
            SettingsDlg settingsDlg = new SettingsDlg();
            settingsDlg.ShowDialog();
        }

        //--- Settings_PropertyChanged -----------------------------------------------
        void Settings_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Units")) _PrinterTypeChanged();
        }

        void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            _ShowTab(true/*(RxGlobals.User.UserType >= EUserType.usr_supervisor)*/, TabAlignment);
            _ShowTab((RxGlobals.User.UserType >= EUserType.usr_service), TabNetwork);
            _ShowTab(RxGlobals.User.UserType >= EUserType.usr_mouvent, TabLog);
        }

        //--- Window_Loaded ------------------------------------------------------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            
        }

        //--- Window_Closed ------------------------------------------
        private void Window_Closed(object sender, EventArgs e)
        {
            RxGlobals.RxInterface.ShutDown();
            
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
                case ELogType.eErrWarn: TabHeaderUnderlineColor = Brushes.Gold; break;
                case ELogType.eErrCont: TabHeaderUnderlineColor = Brushes.Crimson; break;
                case ELogType.eErrStop: TabHeaderUnderlineColor = Brushes.Crimson; break;
                case ELogType.eErrAbort: TabHeaderUnderlineColor = Brushes.Crimson; break;
                default: TabHeaderUnderlineColor = Brushes.Transparent; break;
            }
        }

        //--- Hyperlink_RequestNavigate -------------------------------------------
        private void Hyperlink_RequestNavigate(object sender, RequestNavigateEventArgs e)
        {
            Process.Start(new ProcessStartInfo(e.Uri.AbsoluteUri));
            e.Handled = true;
        }        
        
        
        //--- PrintSystem_PropertyChanged --------------------------
        void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("PrinterType")) _PrinterTypeChanged();
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

           bool showPrintQueueTab = !((RxGlobals.PrintSystem.PrinterType  == EPrinterType.printer_test_table 
                || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide
                || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide_only
				|| RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LH702
                ));

           _ShowTab(showPrintQueueTab, TabPrintQueue);

           PrinterTypeChangedEventArgs eventArgs = new PrinterTypeChangedEventArgs();
           eventArgs.Textile = tx;
           eventArgs.Label = lb;

           PrinterTypeChangedEventHandler(this, eventArgs);

           _ShowPrinterTypeTab(RxGlobals.PrintSystem.PrinterType);

           _ShowPrintSystemTab();
        }

        //--- _activeView -------------------------------
        private void _activeView(TabItem activeItem)
        {
            for (int i=1; i<TabCtrl.Items.Count; i++)
            {
                TabItem item = TabCtrl.Items[i] as TabItem;
                if (item.Name.Equals("TabEvents")) break;
                if (item.Equals(activeItem)
                    || (activeItem.Equals(TabLH702) && item.Equals(TabLB702UV) && RxGlobals.PrintSystem.LH702_simulation)
                    )
                {
                    item.Visibility=Visibility.Visible;
                }
                else item.Visibility=Visibility.Collapsed;
            }
        }

        private void _ShowTab(bool show, TabItem tab)
        {
            try
            {
                if (show)
                {
                   TabCtrl.Items.Add(tab);
                }
                else
                {
                    TabCtrl.Items.Remove(tab);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void _ShowPrinterTypeTab(EPrinterType type)
        {
            foreach (var item in PrinterTypeTabs)
            {
                _ShowTab(false, item);
            }

            switch (type)
            {
                case EPrinterType.printer_LB701:
                    _ShowTab(true, TabLB701);
                    break;

                case EPrinterType.printer_LB702_UV:
                    _ShowTab(true, TabLB702UV);
                    break;

                case EPrinterType.printer_LB702_WB:
                    _ShowTab(true, TabLB702WB);
                    break;

                case EPrinterType.printer_LH702:
                    _ShowTab(true, TabLH702);
                    break;

                case EPrinterType.printer_DP803:
                    _ShowTab(true, TabDP803);
                    break;

                case EPrinterType.printer_TX801:
                case EPrinterType.printer_TX802:
                    _ShowTab(true, TabTex);
                    break;

                case EPrinterType.printer_test_slide:
                case EPrinterType.printer_test_slide_only:
                case EPrinterType.printer_test_table:
                    _ShowTab(true, TabTestTable);
                    break;

                case EPrinterType.printer_cleaf:
                    _ShowTab(true, TabCleaf);
                    break;
                    /*
                case EPrinterType.printer_CB612:
                    // TODO(CB612) Add tab for CB612
                    break;
                    */
            }
        }

        private void _ShowPrintSystemTab()
        {            
            if (RxGlobals.PrintSystem.PrinterType != EPrinterType.printer_CB612)
            {
                PrintSystemView.Visibility = Visibility.Visible;
                PrintSystemExtendedView.Visibility = Visibility.Collapsed;
            }
            else
            {
                PrintSystemView.Visibility = Visibility.Collapsed;
                PrintSystemExtendedView.Visibility = Visibility.Visible;
            }
        }

        private void _InitTabs()
        {
            _ShowTab(false, TabPrintQueue);
            _ShowTab(true, TabEvents);
            _ShowTab(RxGlobals.User.UserType >= EUserType.usr_mouvent, TabLog); // only used for mouvent
            _ShowTab(true, TabPrintSystem);

            _ShowTab(/*(RxGlobals.User.UserType >= EUserType.usr_supervisor)*/ true, TabAlignment);
            
            _ShowTab((RxGlobals.User.UserType >= EUserType.usr_service), TabNetwork);
            
            PrinterTypeTabs = new List<TabItem>()
            {
                TabLB701,
                TabLB702UV,
                TabLB702WB,
                TabLH702,
                TabDP803,
                TabTex,
                TabTestTable,
                TabCleaf,
                // //TODO(CB612) Add tab for CB612
            };
            _ShowPrinterTypeTab(RxGlobals.PrintSystem.PrinterType);
        }


        //--- PrintQueueView_Loaded ---------------------------------------------------
        private void PrintQueueView_Loaded(object sender, RoutedEventArgs e)
        {

        }

        public class PrinterTypeChangedEventArgs : EventArgs
        {
            public bool Textile { get; set; }
            public bool Label { get; set; }
        }

        private void TabCtrl_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            try 
            {
                var selectedTab = TabCtrl.SelectedItem as TabItem;
                SelectedTabName = selectedTab.Name;
            }
            catch(Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }
    }
}
