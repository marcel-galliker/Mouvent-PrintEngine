using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Threading;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.PrintSystemView
{
    public partial class PrintSystemView : UserControl
    {
        private PrintSystem _PrintSystem   = RxGlobals.PrintSystem;

        //--- constructor --------------------------------------------------------
        public PrintSystemView()
        {
            InitializeComponent();
            DataContext = _PrintSystem;
            
            ChillerError.DataContext = RxGlobals.Chiller;
            _PrintSystem.PropertyChanged += _PrintSystem_PropertyChanged;
            RxGlobals.PrinterStatus.PropertyChanged += PrinterStatusChanged;
            RxGlobals.User.PropertyChanged += User_PropertyChanged;
            User_PropertyChanged(null, null);
            RxGlobals.Timer.TimerFct += _timer_Tick;
            OffsetsGrid.DataContext = RxGlobals.PrintSystem;
            EncoderGrid.SetDataContext();
            _PrintSystem.ExpandSettingsPanel = false;
            _PrintSystem.ChillerEnabled = false;
        }

        //--- User_PropertyChanged --------------------------------------
        private void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            Visibility visibility       =  (RxGlobals.User.UserType >= EUserType.usr_service) ? Visibility.Visible : Visibility.Collapsed; 
            PrintSystemPanel.Visibility = visibility;
            visibility = (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_table_seon && RxGlobals.User.UserType >= EUserType.usr_mouvent) ? Visibility.Visible : Visibility.Collapsed;
            FluidSettings.Visibility = visibility;
            FlushCommands.Visibility = visibility;
            ConditionerReset.Visibility = visibility;
        }

        //--- _PrintSystem_PropertyChanged -----------------------------------------------
        private void _PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("ColorCnt") || e.PropertyName.Equals("InkCylindersPerColor"))
            {
                InkSupplyGrid.show_inkSupplies(_PrintSystem.ColorCnt * _PrintSystem.InkCylindersPerColor);
                PrintHeadGrid.show_items((int)(_PrintSystem.ColorCnt * _PrintSystem.HeadsPerColor));
            }
            if (e.PropertyName.Equals("HeadsPerColor"))
            {
                PrintHeadGrid.show_items((int)(_PrintSystem.ColorCnt * _PrintSystem.HeadsPerColor));
            }
            if (e.PropertyName.Equals("CheckedInkSupply") || e.PropertyName.Equals("AllInkSupplies"))
            {
                PrintHeadGrid.show_items((int)(_PrintSystem.ColorCnt * _PrintSystem.HeadsPerColor));
            }
            if (e.PropertyName.Equals("PrinterType"))
            {
                // TODO(CB612) add case for CB612

                StepperGrid.Children.Clear();
                
                switch (RxGlobals.PrintSystem.PrinterType)
                {
                case EPrinterType.printer_cleaf:    
                                                    var stepperGridCleaf = new StepperGridCleaf();
                                                    stepperGridCleaf.SetDataContext();
                                                    StepperGrid.Margin = new Thickness(10, 0, 0, 0);
                                                    StepperGrid.Children.Add(stepperGridCleaf);
                                                    break; 

                case EPrinterType.printer_DP803:    
                                                    var stepperGridDP803 = new StepperGridDP803();
                                                    stepperGridDP803.SetDataContext();
                                                    StepperGrid.Children.Add(stepperGridDP803);
                                                    StepperGrid.Margin = new Thickness(10, 0, 0, 0);
                                                    break;

                case EPrinterType.printer_LB702_UV:
                case EPrinterType.printer_LB702_WB:
                case EPrinterType.printer_LH702:    var stepperGridLB702 = new StepperGridLB702();
                                                    stepperGridLB702.SetDataContext();
                                                    StepperGrid.Children.Add(stepperGridLB702);
                                                    StepperGrid.Margin = new Thickness(10, 0, 0, 0);
                                                    break;

                case EPrinterType.printer_TX801:
                case EPrinterType.printer_TX802:
                case EPrinterType.printer_TX404:
                                                    break;

                default:                           
                                                    var stepperGrid = new StepperGrid();
                                                    stepperGrid.SetDataContext();
                                                    StepperGrid.Children.Add(stepperGrid);
                                                    StepperGrid.Margin = new Thickness(10, 0, 0, 0);
                                                    break;
                }
            }
        }

        //--- PrinterStatusChanged ----------------------------------------
        private void PrinterStatusChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
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

        //--- Purge_Clicked -------------------------------------------------
        private void Purge_Clicked(object sender, RoutedEventArgs e)
        {
        //    if (Button_Purge.IsEnabled) RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, "CMD_SLIDE_TO_PURGE");
        }

        //--- _timer_Tick ---------------------------------------------------
        private void _timer_Tick(int tick)
        {   
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_FLUID_STAT);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_HEAD_STAT);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ENCODER_STAT);
        }

        //--- UserControl_IsVisibleChanged ----------------------------------
        private void UserControl_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
        }

        private void ExpandSettings_Clicked(object sender, RoutedEventArgs e)
        {
            _PrintSystem.ExpandSettingsPanel = !_PrintSystem.ExpandSettingsPanel;
            EncoderGrid.UpdateVisibility(_PrintSystem.PrinterType);
            if (RxGlobals.Encoder[0].Enabled)
            {
                EncoderGrid.Margin = new Thickness(10, 0, 0, 0);
            }
            else
            {
                EncoderGrid.Margin = new Thickness(0);
            }

        }
    }
}
