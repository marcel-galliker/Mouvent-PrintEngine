using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.UsersView;
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
using System.Timers;
using RX_DigiPrint.Views.Settings;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for MouventStatusBar.xaml
    /// </summary>
    public partial class MouventStatusBar : UserControl, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private Timer timeTimer = new Timer(60000);

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

            RxGlobals.User.PropertyChanged += UserType_PropertyChanged;

            Version.Text = "V " + Settings.SettingsDlg.GetVersion();

            UserType_Init();

            SetTime();
            timeTimer.Elapsed += GetNewTime;
            timeTimer.AutoReset = true;
            timeTimer.Start();
        }

        private void GetNewTime(object sender, ElapsedEventArgs e)
        {
            this.Dispatcher.Invoke(() =>
            {
                SetTime();
            });
        }

        private void SetTime()
        {
            System.Globalization.CultureInfo.CurrentCulture.ClearCachedData();
            ActualTime.Text = DateTime.Now.ToString("dd/MM/yy H:mm");
        }

        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        void UserType_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("UserType"))
            {
                RxBindable.Invoke(() =>
                {
                    UserType.Text = RxGlobals.User.UserType.ToString().Substring(4);
                    Logout.Visibility = RxGlobals.User.UserType > EUserType.usr_operator ? Visibility.Visible : Visibility.Hidden;
                });
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

        private void ResetCounters_Click(object sender, RoutedEventArgs e)
        {
            if (RxGlobals.PrinterStatus.PrintState != EPrintState.ps_printing)
            {
                if (MvtMessageBox.YesNo(RX_DigiPrint.Resources.Language.Resources.Reset, RX_DigiPrint.Resources.Language.Resources.ResetCounter, MessageBoxImage.Question, false))
                    RxGlobals.RxInterface.SendCommand(TcpIp.CMD_RESET_CTR);
            }
        }

        private void UserType_Init()
        {
            UserType.Text = RxGlobals.User.UserType.ToString().Substring(4);
            Logout.Visibility = (RxGlobals.User.UserType == EUserType.usr_operator) ? Visibility.Hidden : Visibility.Visible;
            
        }

        private void UserType_Click(object sender, RoutedEventArgs e)
        {
            if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_test_table_seon
            ||  RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_test_slide)
            {
                if (RxGlobals.User.UserType == EUserType.usr_operator) RxGlobals.User.UserType=EUserType.usr_engineer;
                else RxGlobals.User.UserType = EUserType.usr_operator;
            }
            else
            {
                MvtUserLevelManager ulm = new MvtUserLevelManager(SettingsDlg.GetVersion(), RxGlobals.PrinterProperties.Host_Name);
                UserLogin ulw = new UserLogin(ulm);
                ulw.Owner = Window.GetWindow(this);
                if ((bool)ulw.ShowDialog())
                {
                    /* New user type corresponds to the result of the authentication */
                    RxGlobals.User.UserType = (EUserType)ulm.GetLevel();
                }
            }
        }

        private void Logout_Click(object sender, RoutedEventArgs e)
        {
            RxGlobals.User.UserType = EUserType.usr_operator;
        }

        private void ActualTime_MouseDown(object sender, MouseButtonEventArgs e)
        {
            Process timedate = new Process();
            timedate.StartInfo.FileName = "timedate.cpl";
            timedate.Start();
        }
    }
}
