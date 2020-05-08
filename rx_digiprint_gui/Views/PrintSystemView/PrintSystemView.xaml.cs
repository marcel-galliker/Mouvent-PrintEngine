using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;

namespace RX_DigiPrint.Views.PrintSystemView
{
    public partial class PrintSystemView : UserControl
    {
        private PrintSystem _PrintSystem   = RxGlobals.PrintSystem;

        //--- constructor --------------------------------------------------------
        public PrintSystemView()
        {
            InitializeComponent();
          //  DataContext = this;
            DataContext = _PrintSystem;
            RobotButtons.DataContext = RxGlobals.PrinterStatus;
            ChillerError.DataContext = RxGlobals.Chiller;
            _PrintSystem.PropertyChanged += _PrintSystem_PropertyChanged;
            RxGlobals.PrinterStatus.PropertyChanged += PrinterStatusChanged;
            RxGlobals.User.PropertyChanged += User_PropertyChanged;
            User_PropertyChanged(null, null);
            RxGlobals.Timer.TimerFct += _timer_Tick;
        }

        //--- User_PropertyChanged --------------------------------------
        private void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            Visibility visibility       =  (RxGlobals.User.UserType >= EUserType.usr_service) ? Visibility.Visible : Visibility.Collapsed; 
            PrintSystemPanel.Visibility = visibility;
            HeadDistPanel.Visibility    = visibility; 
        }

        //--- _PrintSystem_PropertyChanged -----------------------------------------------
        private void _PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("ColorCnt")) 
            {
                InkSupplyGrid.show_inkSupplies(_PrintSystem.ColorCnt);
                PrintHeadGrid.show_items((int)(_PrintSystem.ColorCnt*_PrintSystem.HeadCnt));
            }

            if (e.PropertyName.Equals("HeadCnt")) 
            {
                PrintHeadGrid.show_items((int)(_PrintSystem.ColorCnt*_PrintSystem.HeadCnt));
            }
            if (e.PropertyName.Equals("CheckedInkSupply") || e.PropertyName.Equals("AllInkSupplies"))
            {
                PrintHeadGrid.show_items((int)(_PrintSystem.ColorCnt*_PrintSystem.HeadCnt));
            }
            if (e.PropertyName.Equals("PrinterType"))
            {
           //     OffsetsGrid.Visibility    = (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_test_slide_only)? Visibility.Collapsed : Visibility.Visible;
                switch(RxGlobals.PrintSystem.PrinterType)
                {
                case EPrinterType.printer_cleaf:    StepperGrid.Children.Clear();
                                                    StepperGrid.Children.Add(new StepperGridCleaf());
                                                    break; 

                case EPrinterType.printer_DP803:    StepperGrid.Children.Clear();
                                                    StepperGrid.Children.Add(new StepperGridDP803());
                                                    break;

                case EPrinterType.printer_LB702_UV:
                case EPrinterType.printer_LB702_WB: 
                case EPrinterType.printer_LH702:    StepperGrid.Children.Clear();
                                                    StepperGrid.Children.Add(new StepperGridLB702());
                                                    break;

                case EPrinterType.printer_TX801:
                case EPrinterType.printer_TX802:    StepperGrid.Children.Clear();
                                                    break;            

                default:                            StepperGrid.Children.Clear();
                                                    StepperGrid.Children.Add(new StepperGrid());
                                                    break;                               
                }
            }            
        }

        //--- PrinterStatusChanged ----------------------------------------
        private void PrinterStatusChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            bool enabled = (RxGlobals.PrinterStatus.PrintState!=EPrintState.ps_printing) && (RxGlobals.PrinterStatus.PrintState!=EPrintState.ps_stopping);
        //    Button_Purge.IsEnabled = enabled;
        //    Button_Wipe.IsEnabled  = enabled;
        }

        //--- Save_Clicked ------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.Stepper.SendStepperCfg();
            _PrintSystem.SendMsg(TcpIp.CMD_SET_PRINTER_CFG);
        }

        //--- Reload_Clicked ------------------------------------------
        private void Reload_Clicked(object sender, RoutedEventArgs e)
        {
            _PrintSystem.SendMsg(TcpIp.CMD_GET_INK_DEF);
            _PrintSystem.SendMsg(TcpIp.CMD_GET_PRINTER_CFG);
            _PrintSystem.SendMsg(TcpIp.CMD_GET_STEPPER_CFG);
        }

        //--- _SetCtrlMode ----------------
        private void _SetCtrlMode(EFluidCtrlMode ctrlMode)
        {
            TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
            msg.no       = -1;
            //msg.ctrlMode = EFluidCtrlMode.ctrl_cap;
            msg.ctrlMode = ctrlMode;
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
        }

        //--- Cap_Clicked -------------------------------------------------
        private void Cap_Clicked(object sender, RoutedEventArgs e)
        {            
            _SetCtrlMode(EFluidCtrlMode.ctrl_cap);
        }

        //--- Wipe_Clicked -------------------------------------------------
        private void Wipe_Clicked(object sender, RoutedEventArgs e)
        {            
            _SetCtrlMode(EFluidCtrlMode.ctrl_wipe);
        }

        //--- WetWipe_Clicked -------------------------------------------------
        private void Vacuum_Clicked(object sender, RoutedEventArgs e)
        {            
            _SetCtrlMode(EFluidCtrlMode.ctrl_vacuum);
        }

        //--- Wash_Clicked -------------------------------------------------
        private void Wash_Clicked(object sender, RoutedEventArgs e)
        {            
            _SetCtrlMode(EFluidCtrlMode.ctrl_wash);
        }

        //--- Purge_Clicked -------------------------------------------------
        private void Purge_Clicked(object sender, RoutedEventArgs e)
        {
        //    if (Button_Purge.IsEnabled) RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, "CMD_SLIDE_TO_PURGE");
        }

        //--- _timer_Tick ---------------------------------------------------
        private void _timer_Tick(int tickNo)
        {    
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_FLUID_STAT);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_HEAD_STAT);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ENCODER_STAT);
        }

        //--- UserControl_IsVisibleChanged ----------------------------------
        private void UserControl_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
        //   if ((bool)e.NewValue) RxGlobals.Timer.TimerFct += _timer_Tick;
        //   else                  RxGlobals.Timer.TimerFct -= _timer_Tick;
        }
    }
}
