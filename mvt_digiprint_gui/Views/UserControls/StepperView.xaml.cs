using RX_Common;
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
        private Image[] _LedRef     = new Image[STEPPER_CNT];
        private Image[] _RefDone    = new Image[STEPPER_CNT];
        private Image[] _LedUp      = new Image[STEPPER_CNT];
        private Image[] _LedPrint   = new Image[STEPPER_CNT];
        private Image[] _LedCap     = new Image[STEPPER_CNT];
        private Image[] _LedWash    = new Image[STEPPER_CNT];
        private Image[] _LedVacuum  = new Image[STEPPER_CNT];
        private Image[] _LedRobRef  = new Image[STEPPER_CNT];

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
            RxGlobals.User.PropertyChanged += User_PropertyChanged;

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
                    Grid.SetRow(_LedCap[i], 6);
                    Grid.SetColumn(_LedCap[i], 1+i);
                    StepperGrid.Children.Add(_LedCap[i]);

                    _LedWash[i] = new Image();
                    Grid.SetRow(_LedWash[i], 7);
                    Grid.SetColumn(_LedWash[i], 1+i);
                    StepperGrid.Children.Add(_LedWash[i]);

                    _LedVacuum[i] = new Image();
                    Grid.SetRow(_LedVacuum[i], 8);
                    Grid.SetColumn(_LedVacuum[i], 1+i);
                    StepperGrid.Children.Add(_LedVacuum[i]);

                    _LedRobRef[i] = new Image();
                    Grid.SetRow(_LedRobRef[i], 10);
                    Grid.SetColumn(_LedRobRef[i], 1+i);
                    StepperGrid.Children.Add(_LedRobRef[i]);

                    RxGlobals.StepperStatus[i].PropertyChanged += Stepper_PropertyChanged;
                }
            }
            User_PropertyChanged(null, null);
            PrinterType_changed();
        }

        //--- Network_CollectionChanged -----------------------------------------------------------
        private void Network_CollectionChanged(object sender,System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            bool[] used = new bool[STEPPER_CNT];
            foreach(NetworkItem item in RxGlobals.Network.List)
            {
                if (item.DeviceType==EDeviceType.dev_stepper && !RxGlobals.PrintSystem.IsCLEAF && item.DeviceNo<=STEPPER_CNT) used[item.DeviceNo-1]=true;
            }
            for (int i=0; i<STEPPER_CNT; i++) _RefDone[i].Visibility = used[i]? Visibility.Visible : Visibility.Collapsed;
        }

        //--- Stepper_PropertyChanged --------------------------------------------
        private void Stepper_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            StepperStatus stat = sender as StepperStatus;

            _LedRobRef[stat.No].Source = (stat.X_in_ref && RxGlobals.StepperStatus[stat.No].ScrewerUsed && RxGlobals.User.UserType == EUserType.usr_mouvent) ? _GreenLedImg : null;

            if(e.PropertyName.Equals("Connected") 
            || e.PropertyName.Equals("CmdRunning") 
            || e.PropertyName.Equals("ClnCmdRunning") 
            || e.PropertyName.Equals("X_in_ref")
    //        || e.PropertyName.Equals("Z_in_ref")
    //        || e.PropertyName.Equals("Z_in_print")
    //        || e.PropertyName.Equals("Z_in_up")
            )
			{ 
                _updateLeds(stat);
			}
        }

        private int _test=0;
        private void Test_Click(object sender,RoutedEventArgs e)
		{
            StepperStatus stat = RxGlobals.StepperStatus[0];
            switch(_test)
			{
                case 0: stat.Connected=true; stat.ClnUsed=true; stat.CmdRunning=0; break;
                case 1: stat.ClnCmdRunning=TcpIp.CMD_ROB_VACUUM; break;
                case 2: stat.ClnVacuumDone=true; stat.ClnCmdRunning=0; break;
                case 3: stat.ClnVacuumDone=false; 
                        _updateLeds(stat);
                        break;
			}
            _test++;
		}

        //--- _setLedImg -----------------------------
        private ImageSource _setLedImg(Visibility visible, uint running, uint cmd, bool done)
		{
            if (visible==Visibility.Visible)
			{
                if (running!=0 && running!=cmd) return null;
                if (done) return _GreenLedImg;
                if (running==cmd) return _GreyLedImg;
			}
            return null; 
		}

        //--- _updateLeds --------------------------------------------------
        private void _updateLeds(StepperStatus stat)
		{                
            bool cmdRunning=false;
            Visibility visible=Visibility.Collapsed;
            bool refDone=RxGlobals.StepperStatus[0].RefDone;

            for (int i=0; i<RxGlobals.StepperStatus.Length; i++)
            {
                if (RxGlobals.StepperStatus[i].Connected)
				{
                    cmdRunning |= RxGlobals.StepperStatus[i].CmdRunning!=0;
                    cmdRunning |= RxGlobals.StepperStatus[i].ClnCmdRunning!=0;

                    if (RxGlobals.StepperStatus[i].RefDone) refDone=true;
                    if (RxGlobals.StepperStatus[i].ClnUsed && RxGlobals.User.UserType==EUserType.usr_mouvent) visible = Visibility.Visible;
				}
            }

            Button_Wash.Visibility      = visible;
            Button_Vacuum.Visibility    = visible;
            Visibility robVisible = visible; 
            if (!RxGlobals.PrintSystem.IsRobotConnected) robVisible=Visibility.Collapsed;
            Button_RefRobot.Visibility  = robVisible;
            Button_Service.Visibility   = robVisible;
            Button_Robot_Zeroing.Visibility = robVisible;

            _RefDone [stat.No].Source = (stat.RefDone)   ? _CheckedImg  : _UncheckedImg;

            _LedRef  [stat.No].Source  = _setLedImg(Visibility.Visible, stat.CmdRunning, TcpIp.CMD_LIFT_REFERENCE,     stat.Z_in_ref);
            _LedUp   [stat.No].Source  = _setLedImg(Visibility.Visible, stat.CmdRunning, TcpIp.CMD_LIFT_UP_POS,        stat.Z_in_up);
            _LedPrint[stat.No].Source  = _setLedImg(Visibility.Visible, stat.CmdRunning, TcpIp.CMD_LIFT_PRINT_POS,     stat.Z_in_print);
            _LedCap  [stat.No].Source  = _setLedImg(Visibility.Visible, stat.CmdRunning, TcpIp.CMD_LIFT_CAPPING_POS,   stat.Z_in_cap);

            _LedWash[stat.No].Source   = _setLedImg(visible, stat.ClnCmdRunning, TcpIp.CMD_ROB_WASH,        stat.ClnWashDone);
            _LedVacuum[stat.No].Source = _setLedImg(visible, stat.ClnCmdRunning, TcpIp.CMD_ROB_VACUUM,      stat.ClnVacuumDone);

            Button_Ref.IsEnabled            = !cmdRunning;
            Button_Up.IsEnabled             = !cmdRunning;
            Button_Print.IsEnabled          = !cmdRunning;
            Button_Wash.IsEnabled           = refDone && !cmdRunning;
            Button_Vacuum.IsEnabled         = refDone && !cmdRunning;
            Button_Cap.IsEnabled            = refDone && !cmdRunning;
            Button_RefRobot.IsEnabled       = refDone && !cmdRunning;
            Button_Service.IsEnabled        = refDone && !cmdRunning;
            Button_Robot_Zeroing.IsEnabled  = refDone && !cmdRunning;
		}

        //--- User_PropertyChanged --------------------------------------
        private void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            foreach (StepperStatus stat in RxGlobals.StepperStatus)
                _updateLeds(stat);
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
            if (e.PropertyName.Equals("PrinterType")) PrinterType_changed();
        }

        private void PrinterType_changed()
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

            RefSensors.Visibility = (RxGlobals.PrintSystem.IsTx || RxGlobals.PrintSystem.IsCLEAF) ? Visibility.Visible : Visibility.Collapsed;
            for (int i=0; i<STEPPER_CNT; i++)
                _RefDone[i].Visibility   = (RxGlobals.PrintSystem.IsTx || RxGlobals.PrintSystem.IsCLEAF) ? Visibility.Collapsed : Visibility.Visible;
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

        //--- LiftReference_clicked -------------------------------------------
        private void LiftReference_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_LIFT_REFERENCE);
            if (SIMU) for(int i=0; i<4; i++) RxGlobals.StepperStatus[i].CmdRunning = TcpIp.CMD_LIFT_REFERENCE;
        }

        //--- LiftUp_clicked -------------------------------------------
        private void LiftUp_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_LIFT_UP_POS);
            if (SIMU) for(int i=0; i<4; i++) RxGlobals.StepperStatus[i].CmdRunning = TcpIp.CMD_LIFT_UP_POS;
        }

        //--- LiftPrint_clicked -------------------------------------------
        private void LiftPrint_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.Stepper.SendStepperCfg();
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_LIFT_PRINT_POS);
            if (SIMU) for(int i=0; i<4; i++) RxGlobals.StepperStatus[i].CmdRunning = TcpIp.CMD_LIFT_PRINT_POS;
        }

        //--- ClnCapping_clicked -------------------------------------------
        private void ClnCapping_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            if (RxMessageBox.YesNo("Capping", "Goto Capping position?",  MessageBoxImage.Question, false))
            {
                bool ClnUsed = RxGlobals.StepperStatus[0].ClnUsed;
                for (int i = 0; i < RxGlobals.StepperStatus.Length; i++)
                {
                    if (RxGlobals.StepperStatus[i].ClnUsed) ClnUsed = true;
                }
                if (ClnUsed || RxGlobals.PrintSystem.IsTx)
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

        //--- Wash_clicked -------------------------------------------
        private void Wash_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
            msg.no = -1;
            msg.ctrlMode = EFluidCtrlMode.ctrl_wash;
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
        }

        //--- Vacuum_clicked -------------------------------------------
        private void Vacuum_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
            msg.no = -1;
            msg.ctrlMode = EFluidCtrlMode.ctrl_vacuum;
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
        }

        //--- Wipe_clicked -------------------------------------------
        private void Wipe_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
            msg.no = -1;
            msg.ctrlMode = EFluidCtrlMode.ctrl_wipe;
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
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

        private void Service_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ROB_SERVICE);
        }

        private void RobotZeroing_clicked(object sender, RoutedEventArgs e)
        {
            _button_active(sender as CheckBox);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_RESET_ALL_SCREWS);
        }
	}
}
