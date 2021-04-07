using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
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
using System.Windows.Input;

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
            CMD_JOG_BWD.DataContext = RxGlobals.Plc;
            CMD_JOG_FWD.DataContext = RxGlobals.Plc;
            Visibility v = Visibility.Collapsed;
            switch (RxGlobals.PrintSystem.PrinterType)
            {
                case EPrinterType.printer_TX801: v = Visibility.Visible; break;
                case EPrinterType.printer_TX802: v = Visibility.Visible; break;
                case EPrinterType.printer_TX404: v = Visibility.Visible; break;
            }
            Button_Wash.Visibility = v;
            Button_Glue.Visibility = v;
            RxGlobals.Timer.TimerFct += _Timer;
        }


        //--- WebIn_Clicked -------------------------------------------------
        private void WebIn_Clicked(object sender, RoutedEventArgs e)
        {
            if (!RxGlobals.Plc.ToWebIn)
            {
                RxGlobals.Plc.ToWebIn = true;
                CMD_WEBIN.IsChecked = true;
                if (RxGlobals.PrintSystem.IsScanning) RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, "CMD_SETUP/CMD_WEBIN");
                else RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PAUSE_PRINTING);
            }
        }

        //--- _Timer ------------------------------------------------------------------
        private void _Timer(int no)
        {
            CMD_WEBIN.IsChecked = RxGlobals.Plc.ToWebIn;

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
        }

        //--- Washing_Clicked -------------------------------------------------
        private void Washing_Clicked(object sender, RoutedEventArgs e)
        {
            if (MvtMessageBox.YesNo(RX_DigiPrint.Resources.Language.Resources.Washing, RX_DigiPrint.Resources.Language.Resources.ConfirmStartWashingTheBelt,  MessageBoxImage.Question, false))
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
            if (MvtMessageBox.YesNo(RX_DigiPrint.Resources.Language.Resources.Gluing, RX_DigiPrint.Resources.Language.Resources.ConfirmStartGluing,  MessageBoxImage.Question, false))
                RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, "CMD_SETUP/CMD_GLUE");
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
            }
            e.Handled = true;
        } 
    }
}
