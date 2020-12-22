using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.PrintQueueView;
using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for CommandButtons.xaml
    /// </summary>
    public partial class MouventCommandButtons : UserControl, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        private void OnPropertyChanged(string name)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged.Invoke(this, new PropertyChangedEventArgs(name));
            }
        }

        private string _PowerButtonToolTip;
        public string PowerButtonToolTip
        {
            get { return _PowerButtonToolTip; }
            set { _PowerButtonToolTip = value; OnPropertyChanged("PowerButtonToolTip"); }
        }

        private bool _ShowPauseButton;
        public bool ShowPauseButton
        {
            get { return _ShowPauseButton; }
            set { _ShowPauseButton = value; OnPropertyChanged("ShowPauseButton"); }
        }

        
        public static Func<bool> StartClicked = null;
       
        public MouventCommandButtons()
        {
            InitializeComponent();
            DataContext = this;
            
            _SetButtonStates();

            RxGlobals.PrinterStatus.PropertyChanged += PrinterStatusChanged;
            RxGlobals.Plc.PropertyChanged += Plc_PropertyChanged;
            RxGlobals.Timer.TimerFct += Timer;
        }

        //--- SetButtonStates -------------------------------------------------
        private void _SetButtonStates()
        {
            //--- checks ---------------------------------------------------------
            // Button_Start.IsChecked  = RxGlobals.PrinterStatus.PrintState==EPrintState.ps_printing;
            if (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_printing)
            {
                if (RxGlobals.Plc.IsReadyForProduction && !RxGlobals.PrinterStatus.Splicing)
                {
                    ShowPauseButton = true;
                }
            }
            else
            {
                ShowPauseButton = false;
            }

            Button_Stop.IsChecked   = RxGlobals.PrinterStatus.PrintState==EPrintState.ps_stopping;
            
            //--- enable ------------------------------------------------------
            Button_Start.IsEnabled  = RxGlobals.Plc.IsReadyForProduction
                                      && !RxGlobals.PrinterStatus.Cleaning
                                      
                                      && 
                                        (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_ready_power 
                                        || (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_pause && (RxGlobals.Encoder[0]!=null && RxGlobals.Encoder[0].CanStart))
                                        || RxGlobals.PrinterStatus.PrintState==EPrintState.ps_printing
                                        );

            
            Button_Stop.IsEnabled   = RxGlobals.Plc.IsReadyForProduction;
            Button_Abort.IsEnabled  = RxGlobals.Plc.IsReadyForProduction;
       //   Button_JogFwd.IsEnabled = RxGlobals.Plc.IsReadyForProduction && !RxGlobals.Plc.IsRunning;
       //   Button_JogBwd.IsEnabled = RxGlobals.Plc.IsReadyForProduction && !RxGlobals.Plc.IsRunning;
       //   Button_Clean.IsEnabled  = (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_pause || RxGlobals.PrinterStatus.PrintState==EPrintState.ps_off);
            
            if (!RxGlobals.PrinterStatus.AllInkSupliesOn) // Button = Power On Button
            {
                Button_Power.IsChecked = false;
                PowerButtonToolTip = "Switch Power On";
                Button_Power.IsEnabled = RxGlobals.PrinterStatus.PrintState == EPrintState.ps_ready_power;
            }
            else // Button = Power Off Button
            {
                Button_Power.IsChecked = true;
                PowerButtonToolTip = "Switch Power Off";
                Button_Power.IsEnabled = true;
            }

            RxGlobals.BtProdState.SetStartBnState(Button_Start.IsEnabled, false, false);
            // RxGlobals.BtProdState.SetPauseBnState(Button_Pause.IsEnabled, Button_Pause.IsChecked, false);
            RxGlobals.BtProdState.SetStopBnState (Button_Stop.IsEnabled,  Button_Stop.IsChecked,  false);
            RxGlobals.BtProdState.SetAbortBnState(Button_Abort.IsEnabled, Button_Abort.IsChecked, false);
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
            if (StartClicked != null)
            {
                bool test = StartClicked();
                if (test == false)
                {
                    return;
                }
            }

            /*orig
            if (StartClicked!=null && !StartClicked()) return;
             * */

#if !DEBUG
            if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf && !(RxGlobals.StepperStatus[0].DripPans_InfeedDOWN && RxGlobals.StepperStatus[0].DripPans_OutfeedDOWN))
            {
                MvtMessageBox.YesNo("Print System", "Drip Pans below the clusters. Move it out before printing", MessageBoxImage.Question, true);
                return;
            }
#endif

            if (ShowPauseButton == true) // pasue button is shown
            {
                RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PAUSE_PRINTING);
                return;
            }

            if (!RxGlobals.PrintSystem.ReadyToPrint()) return;
         
            RxGlobals.PrintQueueView?.SaveAll(); 

            // for (int i = 0; i < 5; i++ )
            {
                RxGlobals.RxInterface.SendCommand(TcpIp.CMD_START_PRINTING);
            }
        }

        //--- Stop_Clicked -------------------------------------------------
        private void Stop_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_STOP_PRINTING);
            RxGlobals.SA_StateMachine.Abort();
        }

        //--- Abort_Clicked -------------------------------------------------
        private void Abort_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ABORT_PRINTING);
        }

        /*
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
        }
        */

        private void Power_Clicked(object sender, RoutedEventArgs e)
        {
            if (!Button_Power.IsChecked) // Switch Power On
            {
                TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd() { no = -1, ctrlMode = EFluidCtrlMode.ctrl_print };
                RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
            }
            else // Switch Power Off
            {
                bool canFlush=false;
                foreach(InkSupply inkSupply in RxGlobals.InkSupply.List)
                    if (inkSupply.InkType!=null) canFlush |= inkSupply.InkType.CanFlush;
                //canFlush=false;
                if (canFlush)
				{
                    FlushWindow wnd = new FlushWindow();
                    wnd.Show();
				}
                else
				{
                    if (MvtMessageBox.YesNo("Print System", "Switch OFF.",  MessageBoxImage.Question, true))
					{
                        TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd(){no=-1, ctrlMode = EFluidCtrlMode.ctrl_shutdown};
                        RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
                        RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ENCODER_UV_OFF);
					}
				}
            }
        }

        //--- Timer -----------------------------------------------------------------
        private void Timer(int no)
        {            
            
        }

        public static event EventHandler SettingsClicked;
        private void TriggerSettingsClicked(EventArgs e)
        {
            if (SettingsClicked != null)
            {
                SettingsClicked(this, e);
            }
        }

        private void Button_Settings_Click(object sender, RoutedEventArgs e)
        {
           SettingsClicked(this, EventArgs.Empty);
        }
	}
}
