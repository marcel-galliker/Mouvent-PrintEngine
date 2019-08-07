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

        //--- _button_active -----------------------------
        private void _button_active(CheckBox button)
        {
            Button_Stop.IsChecked   = false;         
            Button_Ref.IsChecked    = (button==Button_Ref);
            Button_Up.IsChecked     = (button==Button_Up);
            Button_Print.IsChecked  = (button==Button_Print);
            Button_Cap.IsChecked    = (button==Button_Cap);
            Button_DripPans.IsChecked = (button == Button_DripPans);
            Button_Fill.IsChecked = (button == Button_Fill);
            Button_Empty.IsChecked = (button == Button_Empty);
        }

        //--- PrintSystem_PropertyChanged -----------------------------------
        private void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("PrinterType"))
            {
                Visibility visibility;
                
                visibility = (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf) ? Visibility.Visible : Visibility.Collapsed;
                Leds_DripPans.Visibility      = visibility;
                LaserTX.Visibility            = visibility; 
                LaserVal.Visibility           = visibility;

                visibility = (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_DP803) ? Visibility.Visible : Visibility.Collapsed;
                Button_DripPans.Visibility    = visibility;
                Button_DripPansCAP.Visibility = visibility;
                Button_DripPansREF.Visibility = visibility;
                Button_Fill.Visibility        = visibility;
                Button_Empty.Visibility       = visibility;
            }   
        }

        //--- CapStop_clicked -------------------------------------------
        private void CapStop_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CAP_STOP);
        }

        //--- CapReference_clicked -------------------------------------------
        private void CapReference_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CAP_REFERENCE);
        }

        //--- CapUp_clicked -------------------------------------------
        private void CapUp_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CAP_UP_POS);
        }

        //--- CapPrint_clicked -------------------------------------------
        private void CapPrint_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.Stepper.SendStepperCfg();
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CAP_PRINT_POS);
        }

        //--- CapCapping_clicked -------------------------------------------
        private void CapCapping_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            if (RxMessageBox.YesNo("Capping", "Goto Capping position?",  MessageBoxImage.Question, false))
            {   
                RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CAP_CAPPING_POS);
            }
        }

        //--- CapFill_clicked -------------------------------------------
        private void CapFill_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CAP_FILL);
        }

        //--- CapEmpty_clicked -------------------------------------------
        private void CapEmpty_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CAP_EMPTY);
        }

        //--- CapWiping_clicked -------------------------------------------
        private void CapWiping_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CLN_WIPE);
        }

        //--- CapDripPans_clicked -------------------------------------------
        private void CapDripPans_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CLN_DRIP_PANS);
        }

        //--- CapDripPans_clicked -------------------------------------------
        private void CapDripPansCAP_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CLN_DRIP_PANS_CAP);
        }

        //--- CapDripPans_clicked -------------------------------------------
        private void CapDripPansREF_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CLN_DRIP_PANS_REF);
        }
    }
}
