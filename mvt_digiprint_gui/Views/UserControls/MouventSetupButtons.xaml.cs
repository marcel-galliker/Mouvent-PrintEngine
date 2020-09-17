using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.PrintQueueView;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for SetupButtons.xaml
    /// </summary>
    public partial class MouventSetupButtons : UserControl
    {
        public MouventSetupButtons()
        {
            InitializeComponent();
            DataContext = this;
            CMD_JOG_FWD.TouchEnabled = true;
            CMD_JOG_BWD.TouchEnabled = true;
            CMD_WEBIN.DataContext    = RxGlobals.Plc;
            RxGlobals.PrinterStatus.PropertyChanged += PrinterStatusChanged;
            PrinterStatusChanged(null, null);
            RxGlobals.Timer.TimerFct += _Timer;
        }

        EPrintState _printerState = EPrintState.ps_undef;
        private int _WebInStartTimer=0;

        //--- PrinterStatusChanged --------------------------------------------
        private void PrinterStatusChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (_WebInStartTimer==0 && RxGlobals.PrinterStatus.PrintState!=_printerState && RxGlobals.PrintSystem.PrinterType != EPrinterType.printer_test_table_seon)
            {
                if (RxGlobals.PrinterStatus.PrintState != _printerState)
                {
                    _printerState = RxGlobals.PrinterStatus.PrintState;
                    Visibility visible;
                    Visibility invisible;
                    if (RxGlobals.PrintSystem.IsScanning)
                    {
                        visible   = (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_webin) ? Visibility.Visible   : Visibility.Collapsed; 
                        invisible = (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_webin) ? Visibility.Collapsed : Visibility.Visible; 
                    }
                    else
                    {
                        visible   = (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_pause) ? Visibility.Visible   : Visibility.Collapsed; 
                        invisible = (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_pause) ? Visibility.Collapsed : Visibility.Visible; 
                    }

                    CMD_JOG_FWD.Visibility = visible;
                    CMD_JOG_BWD.Visibility = visible;
                    CMD_WEBIN.Visibility   = invisible;
                    if (_WebInStartTimer==0 && CMD_WEBIN.Visibility!=Visibility.Visible) CMD_WEBIN.IsChecked=false;
                //  CMD_WEBIN.IsEnabled    = (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_off || RxGlobals.PrinterStatus.PrintState==EPrintState.ps_ready_power);
                }
           }
           else if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_table_seon)
           {
                CMD_JOG_FWD.Visibility = Visibility.Visible;
                CMD_JOG_BWD.Visibility = Visibility.Visible;
                CMD_WEBIN.Visibility = Visibility.Collapsed;
           }

           Visibility v=Visibility.Collapsed;
           Visibility a = Visibility.Collapsed;
           switch (RxGlobals.PrintSystem.PrinterType)
           {
               case EPrinterType.printer_TX801: v = Visibility.Visible; break;
               case EPrinterType.printer_TX802: v = Visibility.Visible; break;
               case EPrinterType.printer_TX404: v = Visibility.Visible; break;
               case EPrinterType.printer_test_table_seon: a = Visibility.Visible; break;
           }
            Button_Wash.Visibility = v;
            Button_Glue.Visibility = v;
            Button_ClusterNo.Visibility = a;
        }

        //--- WebIn_Clicked -------------------------------------------------
        private void WebIn_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, "CMD_CLEAR_ERROR");
            CMD_WEBIN.IsChecked = true;
            _WebInStartTimer = 4;
            if (RxGlobals.PrintSystem.IsScanning) RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, "CMD_SETUP/CMD_WEBIN");
            else RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PAUSE_PRINTING);
        }

        //--- _Timer ------------------------------------------------------------------
        private void _Timer(int no)
        {
            string val;
            RxGlobals.Plc.RequestVar("Application.GUI_00_001_Main" + "\n"
                + "STA_WEBIN_PREP_FWD" + "\n"
                + "STA_WEBIN_PREP_BWD" + "\n"
                + "STA_WASHING_TIMER" + "\n"
                + "STA_MACHINE_STATE" + "\n");
            {
                val = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_WASHING_TIMER");
                int time = Rx.StrToInt32(val);
                Button_Wash.IsBusy = time > 0;
                Wash_Time.Text = val + "s";
                Wash_Time.Visibility = (time > 0) ? Visibility.Visible : Visibility.Collapsed;
            }

            if (CMD_JOG_FWD.Visibility == Visibility.Visible)
            {
                val = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_WEBIN_PREP_FWD");
                CMD_JOG_FWD.IsBusy = (val != null) && val.Equals("TRUE");
                val = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_WEBIN_PREP_BWD");
                CMD_JOG_BWD.IsBusy = (val != null) && val.Equals("TRUE");
            }

            int state = Rx.StrToInt32(RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_MACHINE_STATE"));
            if (_WebInStartTimer>0)
			{
                if (--_WebInStartTimer==0 && state==3) WebIn_Clicked(CMD_WEBIN, null);
			}
            else if (state==3) CMD_WEBIN.IsChecked=false;
        }

        //--- Clean_Clicked -------------------------------------------------
        private void Clean_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, "CMD_SETUP/CMD_CLEANING");
        }

        //--- Washing_Clicked -------------------------------------------------
        private void Washing_Clicked(object sender, RoutedEventArgs e)
        {
            if (MvtMessageBox.YesNo("Washing", "Start Washing the belt?",  MessageBoxImage.Question, false))
                RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, "CMD_SETUP/CMD_WASHING");
        }

        //--- Warmup_Clicked -------------------------------------------------
        private void Warmup_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, "CMD_SETUP/CMD_WARMUP");
        }

        //--- Gluing_Clicked -------------------------------------------------
        private void Gluing_Clicked(object sender, RoutedEventArgs e)
        {            
            if (MvtMessageBox.YesNo("Gluing", "Start Gluing?",  MessageBoxImage.Question, false))
                RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, "CMD_SETUP/CMD_GLUE");
        }

        //--- CusterNo_Clicked -------------------------------------------------
        private void CusterNo_Clicked(object sender, RoutedEventArgs e)
        {
            SetClusterNo view = new SetClusterNo();
            view.ShowDialog();
        }

        //--- Jog_PreviewMouseDown -------------------------------
        private void Jog_PreviewMouseDown(object sender, MouseButtonEventArgs e) 
        {
            MvtButton button = sender as MvtButton;
            if (button!=null) 
            {
                RxGlobals.Events.AddItem(new LogItem("Mouse down {0}", button.Name)); 
                Debug.WriteLine("PreviewMouseDown", button.Name);
                button.IsChecked = true;
                RxGlobals.Plc.SetVar(button.Name, 1);
                if (RxGlobals.PrintSystem.PrinterType != EPrinterType.printer_test_table_seon) RxGlobals.Plc.SetVar(button.Name, 1);
                else if (button.Name.Equals("CMD_JOG_FWD")) RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TTS_JOG_FWD);
                else if (button.Name.Equals("CMD_JOG_BWD")) RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TTS_JOG_BWD);
            }
            e.Handled = true;
        }

        //--- Jog_PreviewMouseUp ------------------------------------
        private void Jog_PreviewMouseUp(object sender, MouseButtonEventArgs e) 
        {
            MvtButton button = sender as MvtButton;
            if (button!=null)
            {
                Debug.WriteLine("PreviewMouseUp", button.Name);
                button.IsChecked = false;
                RxGlobals.Plc.SetVar(button.Name, 0);
                if (RxGlobals.PrintSystem.PrinterType != EPrinterType.printer_test_table_seon) RxGlobals.Plc.SetVar(button.Name, 0);
                else RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TTS_JOG_STOP);
            }
            e.Handled = true;
        } 
    }
}
