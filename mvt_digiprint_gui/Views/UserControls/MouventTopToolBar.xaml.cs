using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.PrintQueueView;
using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for CommandButtons.xaml
    /// </summary>
    public partial class MouventTopToolBar : UserControl
    {
//        private object _activeButton;

        public Func<bool> StartClicked = null;

        public MouventTopToolBar()
        {
            InitializeComponent();
            DataContext = this;
            TxRobotButton.DataContext = RxGlobals.PrinterStatus;

            _SetButtonStates();
            
            RxGlobals.PrinterStatus.PropertyChanged += PrinterStatusChanged;
            RxGlobals.Plc.PropertyChanged += Plc_PropertyChanged;
            RxGlobals.Timer.TimerFct += Timer;
        }

        //--- SetButtonStates -------------------------------------------------
        private void _SetButtonStates()
        {
            Button_Test.IsChecked = RxGlobals.PrinterStatus.TestMode;
            
            //--- size --------------------------------------------------------
       //   Button_JogFwd.IsEnabled = RxGlobals.Plc.IsReadyForProduction && !RxGlobals.Plc.IsRunning;
       //   Button_JogBwd.IsEnabled = RxGlobals.Plc.IsReadyForProduction && !RxGlobals.Plc.IsRunning;
       //   Button_Clean.IsEnabled  = (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_pause || RxGlobals.PrinterStatus.PrintState==EPrintState.ps_off);

            RxGlobals.BtProdState.send(false);
        }

        //--- PrinterStatusChanged ----------------------------------------
        private void PrinterStatusChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            _SetButtonStates();
        }

        //--- Plc_PropertyChanged ---------------------------------------                                                                           
        private void Plc_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            _SetButtonStates();
        }

        //--- Timer -----------------------------------------------------------------
        private void Timer(int no)
        {            
            
        }

        private void Test_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_cleaf && !(RxGlobals.StepperStatus[0].DripPans_InfeedDOWN && RxGlobals.StepperStatus[0].DripPans_OutfeedDOWN))
            {
                MvtMessageBox.YesNo("Print System", "Drip Pans below the clusters. Move it out before printing", MessageBoxImage.Question, true);
                return;
            }
            PrintQueueAddTest dlg = new PrintQueueAddTest();
            dlg.ShowDialog();
        }

        private void TxRobot_Clicked(object sender, RoutedEventArgs e)
        {
            TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
            msg.no = -1;
            msg.ctrlMode = EFluidCtrlMode.ctrl_cap;
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
        }

        /*
        //--- JogFwd_MouseDown -------------------------------------------------
        private void JogFwd_MouseDown(object sender, MouseButtonEventArgs e)
        {
        //  _activeButton = sender;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_JOG_FWD);      
            e.Handled=true;      
        }

        //--- JogFwd_MouseUp ------------------------------------------------------
        private void JogFwd_MouseUp(object sender, MouseButtonEventArgs e)
        {
        //  _activeButton = null;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_JOG_STOP);
            e.Handled=true;      
        }

        //--- JogBwd_MouseDown ----------------------------------------------------
        private void JogBwd_MouseDown(object sender, MouseButtonEventArgs e)
        {
        //  _activeButton = sender;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_JOG_BWD);
            e.Handled=true;      
        }

        //--- JogBwd_MouseUp ------------------------------------------------------
        private void JogBwd_MouseUp(object sender, MouseButtonEventArgs e)
        {
        //  _activeButton = null;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_JOG_STOP);
            e.Handled=true;      
        }
*/
    }
}
