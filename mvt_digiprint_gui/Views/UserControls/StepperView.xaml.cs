﻿using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for CappingView.xaml
    /// </summary>
    public partial class StepperView : UserControl
    {
        private const bool SIMU=false;

        private const int STEPPER_CNT=4;
        private Image[] _LedRef   = new Image[STEPPER_CNT];
        private Image[] _RefDone  = new Image[STEPPER_CNT];
        private Image[] _LedUp    = new Image[STEPPER_CNT];
        private Image[] _LedPrint = new Image[STEPPER_CNT];
        private Image[] _LedCap   = new Image[STEPPER_CNT];

        private static ImageSource _GreenLedImg;
        private static ImageSource _GreyLedImg;
        private static ImageSource _ErrorLedImg;
        private static ImageSource _UncheckedImg;
        private static ImageSource _CheckedImg;

        public StepperView()
        {
            InitializeComponent();

            DataContext = RxGlobals.StepperStatus;
            RxGlobals.PrintSystem.PropertyChanged    += PrintSystem_PropertyChanged;
            RxGlobals.Network.List.CollectionChanged += Network_CollectionChanged;

            {
                _GreenLedImg  = new BitmapImage(new Uri("..\\..\\Resources\\Bitmaps\\LED_GREEN.ico", UriKind.RelativeOrAbsolute));
                _GreyLedImg   = new BitmapImage(new Uri("..\\..\\Resources\\Bitmaps\\LED_GREY.ico",  UriKind.RelativeOrAbsolute));
                _ErrorLedImg  = new BitmapImage(new Uri("..\\..\\Resources\\Bitmaps\\LED_ERROR.ico", UriKind.RelativeOrAbsolute));
                _UncheckedImg = new BitmapImage(new Uri("..\\..\\Resources\\Bitmaps\\unchecked.ico", UriKind.RelativeOrAbsolute));
                _CheckedImg   = new BitmapImage(new Uri("..\\..\\Resources\\Bitmaps\\checked.ico",   UriKind.RelativeOrAbsolute));

                for (int i=0; i<STEPPER_CNT; i++)
                {
                    _LedRef[i] = new Image();
                    Grid.SetRow(_LedRef[i], 2);
                    Grid.SetColumn(_LedRef[i], 1+i);
                    StepperGrid.Children.Add(_LedRef[i]);

                    _RefDone[i] = new Image();
                    Grid.SetRow(_RefDone[i], 2);
                    Grid.SetColumn(_RefDone[i], 1+i);
                    _RefDone[i].Source = _UncheckedImg;
                    StepperGrid.Children.Add(_RefDone[i]);

                    _LedUp[i] = new Image();
                    Grid.SetRow(_LedUp[i], 3);
                    Grid.SetColumn(_LedUp[i], 1+i);
                    StepperGrid.Children.Add(_LedUp[i]);

                    _LedPrint[i] = new Image();
                    Grid.SetRow(_LedPrint[i], 4);
                    Grid.SetColumn(_LedPrint[i], 1+i);
                    StepperGrid.Children.Add(_LedPrint[i]);

                    _LedCap[i] = new Image();
                    Grid.SetRow(_LedCap[i], 5);
                    Grid.SetColumn(_LedCap[i], 1+i);
                    StepperGrid.Children.Add(_LedCap[i]);

                    RxGlobals.StepperStatus[i].PropertyChanged += Stepper_PropertyChanged;
                }
            }
        }

        //--- Network_CollectionChanged -----------------------------------------------------------
        private void Network_CollectionChanged(object sender,System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            bool[] used = new bool[STEPPER_CNT];
            foreach(NetworkItem item in RxGlobals.Network.List)
            {
                if (item.DeviceType==EDeviceType.dev_stepper) used[item.DeviceNo-1]=true;
            }
            for (int i=0; i<STEPPER_CNT; i++) _RefDone[i].Visibility = used[i]? Visibility.Visible : Visibility.Collapsed;
        }

        //--- Stepper_PropertyChanged --------------------------------------------
        private void Stepper_PropertyChanged(object sender,System.ComponentModel.PropertyChangedEventArgs e)
        {
            StepperStatus stat = sender as StepperStatus;

            if(e.PropertyName.Equals("CmdRunning")) 
            {
                if(stat.CmdRunning==0)
                {
                    _RefDone [stat.No].Source = (stat.RefDone)   ? _CheckedImg  : _UncheckedImg;
                    _LedRef  [stat.No].Source = (stat.Z_in_ref)  ? _GreenLedImg : null;
                    _LedUp   [stat.No].Source = (stat.Z_in_up)   ? _GreenLedImg : null;
                    _LedPrint[stat.No].Source = (stat.Z_in_print)? _GreenLedImg : null;
                    _LedCap  [stat.No].Source = (stat.Z_in_cap)  ? _GreenLedImg : null;

                    bool refDone=RxGlobals.StepperStatus[0].RefDone;
                    for (int i=0; i<RxGlobals.StepperStatus.Length; i++)
                    {
                        if (RxGlobals.StepperStatus[i].CmdRunning==0 && RxGlobals.StepperStatus[i].RefDone) refDone=true;
                    }
                    Button_Up.IsEnabled     = refDone;
                    Button_Print.IsEnabled  = refDone;
                }
                else
                {
                    _LedRef  [stat.No].Source = (stat.CmdRunning==TcpIp.CMD_LIFT_REFERENCE  )? _GreyLedImg : null;
                    _LedUp   [stat.No].Source = (stat.CmdRunning==TcpIp.CMD_LIFT_UP_POS     )? _GreyLedImg : null;
                    _LedPrint[stat.No].Source = (stat.CmdRunning==TcpIp.CMD_LIFT_PRINT_POS  )? _GreyLedImg : null;
                    _LedCap  [stat.No].Source = (stat.CmdRunning==TcpIp.CMD_LIFT_CAPPING_POS)? _GreyLedImg : null;
                }
            }
        }

        //--- _button_active -----------------------------
        private void _button_active(CheckBox button)
        {
            Button_Stop.IsChecked       = false;         
            Button_Ref.IsChecked        = (button == Button_Ref);
            Button_Up.IsChecked         = (button == Button_Up);
            Button_Print.IsChecked      = (button == Button_Print);
            Button_Cap.IsChecked        = (button == Button_Cap);
            Button_DripPans.IsChecked   = (button == Button_DripPans);
            Button_Fill.IsChecked       = (button == Button_Fill);
            Button_Empty.IsChecked      = (button == Button_Empty);
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
                Button_Cap.Visibility         = (RxGlobals.PrintSystem.PrinterType!=EPrinterType.printer_cleaf) ? Visibility.Visible : Visibility.Collapsed;

                visibility = (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf || RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_DP803) ? Visibility.Visible : Visibility.Collapsed;
                Button_DripPans.Visibility    = visibility;

                visibility = (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_DP803) ? Visibility.Visible : Visibility.Collapsed;
                Button_DripPansCAP.Visibility = visibility;
                Button_DripPansREF.Visibility = visibility;
                Button_Fill.Visibility        = visibility;
                Button_Empty.Visibility       = visibility;

                RefSensors.Visibility = (RxGlobals.PrintSystem.IsTx) ? Visibility.Visible : Visibility.Collapsed;
                for (int i=0; i<STEPPER_CNT; i++)
                    _RefDone[i].Visibility   = (RxGlobals.PrintSystem.IsTx) ? Visibility.Collapsed : Visibility.Visible;
            }   
        }

        //--- CapStop_clicked -------------------------------------------
        private void CapStop_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_LIFT_STOP);
            
            if (SIMU)
            {
                for (int i=0; i<4; i++)
                {
                    if (RxGlobals.StepperStatus[i].CmdRunning!=0)
                    {
                        RxGlobals.StepperStatus[i].Z_in_ref     = (RxGlobals.StepperStatus[i].CmdRunning == TcpIp.CMD_LIFT_REFERENCE);
                        RxGlobals.StepperStatus[i].Z_in_up      = (RxGlobals.StepperStatus[i].CmdRunning == TcpIp.CMD_LIFT_UP_POS);
                        RxGlobals.StepperStatus[i].Z_in_print   = (RxGlobals.StepperStatus[i].CmdRunning == TcpIp.CMD_LIFT_PRINT_POS);
                        RxGlobals.StepperStatus[i].Z_in_cap     = (RxGlobals.StepperStatus[i].CmdRunning == TcpIp.CMD_LIFT_CAPPING_POS);
                        RxGlobals.StepperStatus[i].CmdRunning = 0;
                        break;
                    }
                }
            }
        }

        //--- CapReference_clicked -------------------------------------------
        private void CapReference_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_LIFT_REFERENCE);
            if (SIMU) for(int i=0; i<4; i++) RxGlobals.StepperStatus[i].CmdRunning = TcpIp.CMD_LIFT_REFERENCE;
        }

        //--- CapUp_clicked -------------------------------------------
        private void CapUp_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_LIFT_UP_POS);
            if (SIMU) for(int i=0; i<4; i++) RxGlobals.StepperStatus[i].CmdRunning = TcpIp.CMD_LIFT_UP_POS;
        }

        //--- CapPrint_clicked -------------------------------------------
        private void CapPrint_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.Stepper.SendStepperCfg();
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_LIFT_PRINT_POS);
            if (SIMU) for(int i=0; i<4; i++) RxGlobals.StepperStatus[i].CmdRunning = TcpIp.CMD_LIFT_PRINT_POS;
        }

        //--- CapCapping_clicked -------------------------------------------
        private void CapCapping_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            if (RxMessageBox.YesNo("Capping", "Goto Capping position?",  MessageBoxImage.Question, false))
            {   
                if (RxGlobals.StepperStatus[0].RobotUsed)
                {
                    TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
                    msg.no       = -1;
                    msg.ctrlMode = EFluidCtrlMode.ctrl_cap;
                    RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
                }
                else
                    RxGlobals.RxInterface.SendCommand(TcpIp.CMD_LIFT_CAPPING_POS);
            }
            if (SIMU) for(int i=0; i<4; i++) RxGlobals.StepperStatus[i].CmdRunning = TcpIp.CMD_LIFT_CAPPING_POS;
        }

        //--- CapUp_clicked -------------------------------------------
        private void CapRefRobot_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ROB_REFERENCE);
        }

        //--- CapFill_clicked -------------------------------------------
        private void CapFill_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_LIFT_FILL);
        }

        //--- CapEmpty_clicked -------------------------------------------
        private void CapEmpty_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_LIFT_EMPTY);
        }

        //--- CapWiping_clicked -------------------------------------------
        private void CapWiping_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ROB_WIPE);
        }

        //--- CapDripPans_clicked -------------------------------------------
        private void CapDripPans_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ROB_DRIP_PANS);
        }

        //--- CapDripPansCAP_clicked -------------------------------------------
        private void CapDripPansCAP_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ROB_DRIP_PANS_CAP);
        }

        //--- CapDripPansREF_clicked -------------------------------------------
        private void CapDripPansREF_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ROB_DRIP_PANS_REF);
        }
    }
}