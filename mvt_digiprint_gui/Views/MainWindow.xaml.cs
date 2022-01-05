using Dragablz;
using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.Settings;
using RX_DigiPrint.Views.UserControls;
using RX_DigiPrint.Views;
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
            RestService.Start();

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

            MainNotConnected.DataContext = RxGlobals.RxInterface;
            PrintSystemState.DataContext = RxGlobals.PrintSystem;
            TabCtrl.DataContext = this;

            RxGlobals.PrinterStatus.ErrorTypeChangedEvent += _ErrorTypeChanged;
            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
            
            RxGlobals.Settings.PropertyChanged += Settings_PropertyChanged;

            MouventCommandButtons.SettingsClicked += new EventHandler(CommandButtonsSettingsClicked);

            RxGlobals.User.PropertyChanged += User_PropertyChanged;
            _UserTypeChanged();

            _ShowTab(false, TabLH702, AddLocationHint.After);
            _ShowTab(false, SiemensHMI, AddLocationHint.Last);
        }

        
        void CommandButtonsSettingsClicked(object sender, EventArgs e)
        {
            SettingsDlg settingsDlg = new SettingsDlg();
            settingsDlg.Owner = Window.GetWindow(this);
            settingsDlg.ShowDialog();
        }

        //--- Settings_PropertyChanged -----------------------------------------------
        void Settings_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Units")) _PrinterTypeChanged();
        }

        void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("UserType"))  _UserTypeChanged();

        }

        private EUserType _UserType = EUserType.usr_undef;
        private void _UserTypeChanged()
		{
            if (RxGlobals.User.UserType!=_UserType)
			{
                _ShowTab((RxGlobals.User.UserType >= EUserType.usr_maintenance), TabLog, AddLocationHint.Last);
                _ShowTab((RxGlobals.User.UserType >= EUserType.usr_maintenance), TabNetwork, AddLocationHint.Last);
                _ShowTab((RxGlobals.User.UserType >= EUserType.usr_maintenance), TabUsers, AddLocationHint.Last);             
                _UserType = RxGlobals.User.UserType;
			}
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
                case ELogType.eErrWarn: TabHeaderUnderlineColor  = Rx.BrushWarn;  break;
                case ELogType.eErrCont: TabHeaderUnderlineColor  = Rx.BrushError; break;
                case ELogType.eErrStop: TabHeaderUnderlineColor  = Rx.BrushError; break;
                case ELogType.eErrAbort: TabHeaderUnderlineColor = Rx.BrushError; break;
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
            bool tx = RxGlobals.PrintSystem.IsTx;
            bool lb = RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LB701 
                ||    RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LB702_UV
                ||    RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LB702_WB
                || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LB703_UV
                || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LH702
                ||    RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_DP803;
           
            bool pq = (lb||tx); // &&  RxGlobals.PrintSystem.PrinterType!=EPrinterType.printer_LH702;

           PrinterTypeChangedEventArgs eventArgs = new PrinterTypeChangedEventArgs();
           eventArgs.Textile = tx;
           eventArgs.Label = lb;

           PrinterTypeChangedEventHandler(this, eventArgs);

           _ShowPrinterTypeTab(RxGlobals.PrintSystem.PrinterType);

           _ShowPrintSystemTab();

           _ShowTab(pq, TabPrintQueue, AddLocationHint.First);
        }

        private void _ShowTab(bool show, TabItem tab, AddLocationHint hint)
        {
            TabCtrl.AddLocationHint = hint;
            if (!show) TabCtrl.Items.Remove(tab);
            else if (!TabCtrl.Items.Contains(tab)) TabCtrl.Items.Add(tab);
        }

        private EPrinterType _PrinterType = EPrinterType.printer_undef;
        private void _ShowPrinterTypeTab(EPrinterType type)
        {
            if (type!=_PrinterType)
			{
                switch (type)
                {
                    case EPrinterType.printer_LB701:
                        MachineName.Text="LB 701";
					    TabMachine.Content = new LB701UVView.LB701UVView();
                        break;

                    case EPrinterType.printer_LB702_UV:
                        MachineName.Text="LB 702 UV";
					    TabMachine.Content = new LB702UVView.LB702UV_View();
                        break;

                    case EPrinterType.printer_LB702_WB:
                        MachineName.Text="LB 702 WB";
					    TabMachine.Content = new LB702WBView.LB702WBView();
                        break;

                    case EPrinterType.printer_LB703_UV:
                        MachineName.Text = "LB 703 UV";
                        TabMachine.Content = new LB702UVView.LB702UV_View();
                        break;

                    case EPrinterType.printer_LH702:
                        MachineName.Text="LB 702 UV";
					    TabMachine.Content = new LB702UVView.LB702UV_View();
                        break;

                    case EPrinterType.printer_DP803:
                        MachineName.Text="DP 803";
					    TabMachine.Content = new DP803View.DP803View();
                        break;

                    case EPrinterType.printer_TX801:
                        MachineName.Text="TX 801";
					    TabMachine.Content = new TexView.TexView();
                        break;

                    case EPrinterType.printer_TX802:
                        MachineName.Text="TX 802";
					    TabMachine.Content = new TexView.TexView();
                        break;

                    case EPrinterType.printer_TX404:
                        MachineName.Text = "TX 404";
                        TabMachine.Content = new TexView.TexView();
                        break;

                    case EPrinterType.printer_test_slide:
                    case EPrinterType.printer_test_slide_only:
                    case EPrinterType.printer_test_table:
                        MachineName.Text="Test Table";
                        TabMachine.Content = new TestTableView.TestTableView();
                        break;

                    case EPrinterType.printer_test_table_seon:
                        MachineName.Text = "Test Table Seon";
                        TabMachine.Content = new TestTableSeonView.TestTableSeonView();
                        break;

                    case EPrinterType.printer_cleaf:
                        MachineName.Text="CLEAF";
					    TabMachine.Content = new CleafView.CleafView();
                        break;

                        /*
                    case EPrinterType.printer_CB612:
                        // TODO(CB612) Add tab for CB612
                        break;
                        */
                }

                var act=TabCtrl.SelectedItem;
                TabCtrl.SelectedItem=TabPrintQueue;
                if (type==EPrinterType.printer_LH702)
				{
                    _ShowTab(true, TabLH702, AddLocationHint.After);                    
                    _ShowTab(false, TabMachine, AddLocationHint.After);
                    _ShowTab(!RxGlobals.PrintSystem.isHybrid, SiemensHMI, AddLocationHint.Last);
                }
                else
				{
                    _ShowTab(true , TabMachine, AddLocationHint.After);
                    _ShowTab(false, TabLH702,   AddLocationHint.After);
                    _ShowTab(false, SiemensHMI, AddLocationHint.Last);
                }
                TabCtrl.SelectedItem=act;

                _PrinterType = type;
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
                if (selectedTab!=null) SelectedTabName = selectedTab.Name;
            }
            catch(Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }
	}
}
