using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.PrintSystemExtendedView
{
    /// <summary>
    /// Interaction logic for PrintHeadView.xaml
    /// </summary>

    public partial class PrintHeadView : UserControl, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }
        
        private int _printHeadIndex;
        public int PrintHeadIndex
        {
            get
            {
                return _printHeadIndex;
            }
            set
            {
                _printHeadIndex = value; OnPropertyChanged("PrintHeadIndex");
            }
        }

        public PrintHeadView()
        {
            InitializeComponent();

            RxGlobals.Chiller.PropertyChanged += Chiller_PropertyChanged;
            RxGlobals.User.PropertyChanged += User_PropertyChanged;
            User_PropertyChanged(null, null);
            Chiller_PropertyChanged(null, null);

            Visibility visible = (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide_only) ? Visibility.Visible : Visibility.Collapsed;
            Visibility collapsed = (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide_only) ? Visibility.Collapsed : Visibility.Visible;
            Button_Print.Visibility = visible;
            Button_PurgeHard.Visibility = collapsed;
            Button_PurgeSoft.Visibility = collapsed;
            Button_PurgeMicro.Visibility = collapsed;
            visible = (RxGlobals.StepperStatus[0].RobotUsed) ? Visibility.Visible : Visibility.Collapsed;
            Button_Purge4Ever.Visibility = visible;
        }

        private void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            Visibility visibility = (RxGlobals.User.UserType >= EUserType.usr_service) ? Visibility.Visible : Visibility.Collapsed;
            ServiceGrid.Visibility = visibility;
        }

        private void Chiller_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            HeadStat stat = DataContext as HeadStat;
            if (stat == null || stat.Name == null || stat.Name.StartsWith("?"))
                CmdButton.IsEnabled = false;
            else CmdButton.IsEnabled = RxGlobals.Chiller.Running;
        }

        public void SetPrintHeadIndex(int index)
        {
            PrintHeadIndex = index;
        }

        private void PrintHeadViewDataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            HeadStat stat = DataContext as HeadStat;
            if (stat != null) stat.PropertyChanged += StatusPropertyChanged;
        }

        private void StatusPropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Name")) ChillerPropertyChanged(this, null);
        }

        private void ChillerPropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            HeadStat stat = DataContext as HeadStat;
            if (stat == null || stat.Name == null || stat.Name.StartsWith("?"))
            {
                CmdButton.IsEnabled = false;
            }
            else
            {
                CmdButton.IsEnabled = RxGlobals.Chiller.Running;
            }
        }

        private void Command_Clicked(object sender, RoutedEventArgs e)
        {
            HeadStat stat = DataContext as HeadStat;
            if (stat == null) return;

            if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide_only || RxGlobals.User.UserType == EUserType.usr_mouvent)
                Button_Print.Visibility = Button_Off.Visibility = Visibility.Visible;
            else Button_Print.Visibility = Button_Off.Visibility = Visibility.Collapsed;

            CmdPopup.Open(CmdButton);
        }

        private void _command(string name, EFluidCtrlMode cmd)
        {
            HeadStat stat = DataContext as HeadStat;
            if (stat != null)
            {
                //--- do this also for other printers? --------------
                if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_cleaf)
                {
                    if (name != null && !RxGlobals.StepperStatus[0].X_in_cap) // when slide not under printhead
                    {
                        if (!MvtMessageBox.YesNo(name, "Printhead NOT in capping position. Continue?", MessageBoxImage.Question, true)) return;
                    }
                }

                TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
                msg.no = stat.HeadNo;
                msg.ctrlMode = cmd;

                RxGlobals.RxInterface.SendMsg(TcpIp.CMD_HEAD_FLUID_CTRL_MODE, ref msg);
            }
            CmdPopup.IsOpen = false;
        }

        private void OFF_Clicked(object sender, RoutedEventArgs e) { _command(null, EFluidCtrlMode.ctrl_off); }
        private void Print_Clicked(object sender, RoutedEventArgs e) { _command(null, EFluidCtrlMode.ctrl_print); }
        private void PurgeSoft_Clicked(object sender, RoutedEventArgs e) { _command("Purge", EFluidCtrlMode.ctrl_purge_soft); }
        private void PurgeMicro_Clicked(object sender, RoutedEventArgs e) { _command("Purge", EFluidCtrlMode.ctrl_purge_soft); }
        private void Purge_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide_only)
                _command("Purge", EFluidCtrlMode.ctrl_purge_step2);
            else
                _command("Purge", EFluidCtrlMode.ctrl_purge);
        }
        private void PurgeHard_Clicked(object sender, RoutedEventArgs e) { _command("Purge", EFluidCtrlMode.ctrl_purge_hard); }
        private void Purge4Ever_Clicked(object sender, RoutedEventArgs e) { _command("Purge", EFluidCtrlMode.ctrl_purge4ever); }
        private void Flush_Clicked(object sender, RoutedEventArgs e) { _command("Flush", EFluidCtrlMode.ctrl_flush_night); }
        private void Wipe_Clicked(object sender, RoutedEventArgs e) { _command(null, EFluidCtrlMode.ctrl_wipe); }

    
    }
}
