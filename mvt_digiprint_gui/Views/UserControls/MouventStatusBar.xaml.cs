using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for MouventStatusBar.xaml
    /// </summary>
    public partial class MouventStatusBar : UserControl, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public MouventStatusBar()
        {
            InitializeComponent();

            PrinterStatus.DataContext = RxGlobals.RxInterface;
            
            RxGlobals.PrinterStatus.LogType = ELogType.eErrStop;
            RxGlobals.RxInterface.PropertyChanged += OnRxInterfacePropertyChanged;
            RxGlobals.PrinterProperties.PropertyChanged += PrinterProperties_PropertyChanged;
            Counters.DataContext = RxGlobals.PrinterStatus;
            Speed.DataContext = RxGlobals.PrinterStatus;

            MainWindow.PrinterTypeChangedEventHandler += new EventHandler(UpdateCounters);
            Counters.Visibility = Visibility.Hidden;
            Speed.Visibility = Counters.Visibility;

            Version.Text = "V " + Settings.SettingsDlg.GetVersion();

            UserType_Init();
        }

        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        void UpdateCounters(object sender, EventArgs eventArgs)
        {
            RX_DigiPrint.Views.MainWindow.PrinterTypeChangedEventArgs e =
                (RX_DigiPrint.Views.MainWindow.PrinterTypeChangedEventArgs)eventArgs;

            Counters.Visibility = (e.Textile || e.Label) ? Visibility.Visible : Visibility.Collapsed;
            Speed.Visibility = Counters.Visibility;

            //--- update the speed and counters ------------------------
            CUnit unit = new CUnit("m");
            if (e.Textile) CounterUnit1.Text = unit.Name + "/h";
            else CounterUnit1.Text = unit.Name + "/min";

            CounterUnit2.Text = unit.Name;
            CounterUnit3.Text = unit.Name;
            
            Counters.DataContext = null;
            Counters.DataContext = RxGlobals.PrinterStatus;

            Counters.Visibility = Visibility.Visible;
            Speed.Visibility = Counters.Visibility;
        }


        void PrinterProperties_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Host_Name"))
            {
                RxBindable.Invoke(() =>
                {
                    PrinterName.Text = RxGlobals.PrinterProperties.Host_Name;
                    PrinterName.InvalidateVisual();
                });
            }
        }

        //--- Window_Loaded ------------------------------------------------------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            PrinterStatus.Online = RxGlobals.RxInterface.Connected;
        }

       //--- OnRxInterfacePropertyChanged ---------------------------------------------
        private void OnRxInterfacePropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Connected"))
            {
                PrinterStatus.Dispatcher.Invoke(() => PrinterStatus.Online = RxGlobals.RxInterface.Connected);
            }
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            RxGlobals.Bluetooth.ShutDown();
        }

        private void ResetCounters_Click(object sender, RoutedEventArgs e)
        {
            if (RxGlobals.PrinterStatus.PrintState != EPrintState.ps_printing)
            {
                if (MvtMessageBox.YesNo("Reset", "Reset counter", MessageBoxImage.Question, false))
                    RxGlobals.RxInterface.SendCommand(TcpIp.CMD_RESET_CTR);
            }
        }

        private void UserType_Init()
		{
            UserType.DataContext    = RxGlobals.User;
            UserType.ItemsSource    = new EN_UserTypeList();
            UserType.SelectedValue  = RxGlobals.User.UserType;
		}

		private void UserType_SelectionChanged(object sender,SelectionChangedEventArgs e)
		{
            RxGlobals.User.UserType = (EUserType)UserType.SelectedValue;
		}

    }
}
