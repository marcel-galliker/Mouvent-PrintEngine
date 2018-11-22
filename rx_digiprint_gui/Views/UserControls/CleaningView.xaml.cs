using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for CappingView.xaml
    /// </summary>
    public partial class CleaningView : UserControl
    {
        public CleaningView()
        {
            InitializeComponent();

            DataContext = RxGlobals.TestTableStatus;
        //  PrintHeight.DataContext = RxGlobals.Stepper;
        //  PrintHeight.Visibility  = Visibility.Collapsed;
            RxGlobals.PrintSystem.PropertyChanged +=PrintSystem_PropertyChanged;
        }

        //--- PrintSystem_PropertyChanged -----------------------------------
        private void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("PrinterType"))
            {
                Button_Wipe.Visibility = (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf 
                                       || RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_LB702)? 
                                        Visibility.Visible:Visibility.Collapsed;

                LaserTX.Visibility = LaserVal.Visibility = (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf) ? Visibility.Visible : Visibility.Collapsed;
            }   
        }

        //--- CapStop_clicked -------------------------------------------
        private void CapStop_clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CAP_STOP);
        }

        //--- CapReference_clicked -------------------------------------------
        private void CapReference_clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CAP_REFERENCE);
        }

        //--- CapUp_clicked -------------------------------------------
        private void CapUp_clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CAP_UP_POS);
        }

        //--- CapPrint_clicked -------------------------------------------
        private void CapPrint_clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.Stepper.SendStepperCfg();
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CAP_PRINT_POS);
        }

        //--- CapCapping_clicked -------------------------------------------
        private void CapCapping_clicked(object sender, RoutedEventArgs e)
        {
            if (RxMessageBox.YesNo("Capping", "Goto Capping position?",  MessageBoxImage.Question, false))
            {   
                RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CAP_CAPPING_POS);
            }
        }

        //--- CapWiping_clicked -------------------------------------------
        private void CapWiping_clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CLN_WIPE);
        }
    }
}
