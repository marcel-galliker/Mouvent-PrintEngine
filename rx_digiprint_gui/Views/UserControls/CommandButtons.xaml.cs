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
    public partial class CommandButtons : UserControl
    {
        private double _ButtonHeight;
//        private object _activeButton;

        public Func<bool> StartClicked = null;

        public CommandButtons()
        {
            InitializeComponent();
            DataContext = this;
            
            _ButtonHeight = Button_Pause.Height;
            _SetButtonStates();
            
            Button_PrintRelease.Visibility = Visibility.Collapsed;

            Button_UV.DataContext = RxGlobals.UvLamp;

            RxGlobals.PrinterStatus.PropertyChanged += PrinterStatusChanged;
            RxGlobals.Plc.PropertyChanged += Plc_PropertyChanged;
            RxGlobals.Timer.TimerFct += Timer;
        }

        //--- SetButtonStates -------------------------------------------------
        private void _SetButtonStates()
        {
            double small = _ButtonHeight;
            double large = 1.5*_ButtonHeight;
            int i;

            for (i=0; i<ButtonsPanel.Children.Count; i++)
            {
                RxButton button = ButtonsPanel.Children[i] as RxButton;
                if (button!=null) button.Height = small;
            }

            //--- size --------------------------------------------------------
            Button_Pause.Height     = (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_printing)? large : small;
            Button_Stop.Height      = ( RxGlobals.PrinterStatus.PrintState==EPrintState.ps_printing 
                                     || RxGlobals.PrinterStatus.PrintState==EPrintState.ps_stopping
                                 //  || RxGlobals.PrinterStatus.PrintState==EPrintState.ps_goto_pause
                                     || RxGlobals.PrinterStatus.PrintState==EPrintState.ps_pause
                                      )? large : small;
            Button_Abort.Height     = (RxGlobals.PrinterStatus.PrintState ==EPrintState.ps_printing
                                     || RxGlobals.PrinterStatus.PrintState==EPrintState.ps_stopping
                                     || RxGlobals.PrinterStatus.PrintState==EPrintState.ps_goto_pause
                                     || RxGlobals.PrinterStatus.PrintState==EPrintState.ps_pause            
                                      )? large : small;   

            //--- checks ---------------------------------------------------------
            Button_Start.IsChecked  = RxGlobals.PrinterStatus.PrintState==EPrintState.ps_printing;
            Button_Stop.IsChecked   = RxGlobals.PrinterStatus.PrintState==EPrintState.ps_stopping;
            Button_Test.IsChecked   = RxGlobals.PrinterStatus.TestMode;
            
            //--- enable ------------------------------------------------------
            Button_Start.IsEnabled  = RxGlobals.Plc.IsReadyForProduction 
                                      && !RxGlobals.PrinterStatus.Cleaning
                                      && 
                                        (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_ready_power 
                                        || RxGlobals.PrinterStatus.PrintState==EPrintState.ps_pause 
                                        || RxGlobals.PrinterStatus.PrintState==EPrintState.ps_printing
                                        );

            Button_Pause.IsEnabled  = RxGlobals.Plc.IsReadyForProduction && !RxGlobals.PrinterStatus.Splicing;
            Button_Stop.IsEnabled   = RxGlobals.Plc.IsReadyForProduction;
            Button_Abort.IsEnabled  = RxGlobals.Plc.IsReadyForProduction;
       //   Button_JogFwd.IsEnabled = RxGlobals.Plc.IsReadyForProduction && !RxGlobals.Plc.IsRunning;
       //   Button_JogBwd.IsEnabled = RxGlobals.Plc.IsReadyForProduction && !RxGlobals.Plc.IsRunning;
       //   Button_Clean.IsEnabled  = (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_pause || RxGlobals.PrinterStatus.PrintState==EPrintState.ps_off);
            Button_On.IsEnabled     = !RxGlobals.PrinterStatus.AllInkSupliesOn && RxGlobals.PrinterStatus.PrintState==EPrintState.ps_ready_power;
            Button_Off.IsEnabled    = !RxGlobals.PrinterStatus.AllInkSupliesOff;

            RxGlobals.BtProdState.SetStartBnState(Button_Start.IsEnabled, Button_Start.IsChecked, Button_Start.Height==large);
            RxGlobals.BtProdState.SetPauseBnState(Button_Pause.IsEnabled, Button_Pause.IsChecked, Button_Pause.Height==large);
            RxGlobals.BtProdState.SetStopBnState (Button_Stop.IsEnabled,  Button_Stop.IsChecked,  Button_Stop.Height==large);
            RxGlobals.BtProdState.SetAbortBnState(Button_Abort.IsEnabled, Button_Abort.IsChecked, Button_Abort.Height==large);
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

        //--- Start_Clicked ------------------------------------------------
        private void Start_Clicked(object sender, RoutedEventArgs e)
        {
            if (StartClicked!=null && !StartClicked()) return;
            #if !DEBUG
            if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf && !(RxGlobals.TestTableStatus.DripPans_InfeedDOWN && RxGlobals.TestTableStatus.DripPans_OutfeedDOWN))
            {
                RxMessageBox.YesNo("Print System", "Drip Pans below the clusters. Move it out before printing", MessageBoxImage.Question, true);
                return;
            }
            #endif

            if (!RxGlobals.PrinterStatus.AllInkSupliesOn)
            {
                    if (RxMessageBox.YesNo("Print System", "Some ink supplies are OFF. Switch them ON.",  MessageBoxImage.Question, true))
                    On_Clicked(null, null);
            }

            if (RxGlobals.UvLamp.Visible==Visibility.Visible && !RxGlobals.UvLamp.Ready)
            {
                if (!RxMessageBox.YesNo("UV Lamp", "The UV Lamp is NOT READY.\n\nStart Printing?",  MessageBoxImage.Question, false))
                    return;
            }
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_START_PRINTING);
        }

        //--- Pause_Clicked -------------------------------------------------
        private void Pause_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PAUSE_PRINTING);
        }

        //--- Stop_Clicked -------------------------------------------------
        private void Stop_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_STOP_PRINTING);
        }

        //--- Abort_Clicked -------------------------------------------------
        private void Abort_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ABORT_PRINTING);
        }

        //--- Test_Clicked -------------------------------------------------
        private void Test_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf && !(RxGlobals.TestTableStatus.DripPans_InfeedDOWN && RxGlobals.TestTableStatus.DripPans_OutfeedDOWN))
            {
                RxMessageBox.YesNo("Print System", "Drip Pans below the clusters. Move it out before printing", MessageBoxImage.Question, true);
                return;
            }
            PrintQueueAddTest dlg = new PrintQueueAddTest();     
            dlg.ShowDialog();      
        }

        //--- On_Clicked -------------------------------------------------
        private void On_Clicked(object sender, RoutedEventArgs e)
        {
            TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd(){no=-1, ctrlMode = EFluidCtrlMode.ctrl_print};
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
        }

        //--- Off_Clicked -------------------------------------------------
        private void Off_Clicked(object sender, RoutedEventArgs e)
        {
            FlushWindow wnd = new FlushWindow();
            wnd.Show();
            /*
            if (RxMessageBox.YesNo("Print System", "Shut Down the printer?",  MessageBoxImage.Question, false))
            {
//                TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd(){no=-1, ctrlMode = EFluidCtrlMode.ctrl_shutdown};
                TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd(){no=-1, ctrlMode = EFluidCtrlMode.ctrl_off};
                RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
                RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ENCODER_UV_OFF);
            }
            */
        }

        //--- UV_Clicked ------------------------------------------------------------------------
        private void UV_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxGlobals.UvLamp.On || RxGlobals.UvLamp.Ready)
            {
                if ((RxGlobals.UvLamp.ActionStr==null || !RxGlobals.UvLamp.ActionStr.Equals("OFF")) &&  RxMessageBox.YesNo("UV Dryer", "Switch OFF",  MessageBoxImage.Question, false))
                {
                    RxGlobals.UvLamp.SwitchOff();
                }
            }
            else if (!RxGlobals.UvLamp.Busy) RxGlobals.UvLamp.SwitchOn();
        }

        //--- Timer -----------------------------------------------------------------
        private void Timer(int no)
        {            
            string val="####";                
            
            if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf)
            {
                RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_GET_VAR, "Application.GUI_00_001_Main"+"\n"+ "STA_PRINT_RELEASE"+"\n");
                val = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_PRINT_RELEASE");
            }
            if (val!=null && val.Equals("FALSE")) Button_PrintRelease.Visibility = Visibility.Visible;
            else                                  Button_PrintRelease.Visibility = Visibility.Collapsed;
            
            /*
            if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf)
            {
                Button_PrintRelease.Visibility = Visibility.Visible;
                RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_GET_VAR, "Application.GUI_00_001_Main"+"\n"+ "STA_PRINT_RELEASE"+"\n");
                val = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_PRINT_RELEASE");
                if (val!=null && val.Equals("FALSE")) Button_PrintRelease.Background = Brushes.Red;
                else                                  Button_PrintRelease.Background = Brushes.LightGreen;
            }
            else Button_PrintRelease.Visibility = Visibility.Collapsed;
            */
        }

        //--- PrintRelease_Clicked --------------------------------------------
        private void PrintRelease_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxMessageBox.YesNo("Ready to Print", "Is the machine ready to print?\nNo Splices in the machine?",  MessageBoxImage.Question, false))
                RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, "CMD_PRINT_RELEASE");
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
         * */
    }
}
