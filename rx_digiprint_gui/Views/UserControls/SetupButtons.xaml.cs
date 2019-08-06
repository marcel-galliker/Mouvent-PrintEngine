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
    public partial class SetupButtons : UserControl
    {
        public SetupButtons()
        {
            InitializeComponent();
            DataContext = this;
            CMD_JOG_FWD.TouchEnabled = true;
            CMD_JOG_BWD.TouchEnabled = true;
            RxGlobals.PrinterStatus.PropertyChanged += PrinterStatusChanged;
            PrinterStatusChanged(null, null);
        }

        EPrintState _printerState = EPrintState.ps_undef;

        //--- PrinterStatusChanged --------------------------------------------
        private void PrinterStatusChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (RxGlobals.PrinterStatus.PrintState!=_printerState)
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
                }
           }
           

           Visibility v=Visibility.Collapsed;
           switch (RxGlobals.PrintSystem.PrinterType)
           {
               case EPrinterType.printer_TX801: v = Visibility.Visible; break;
               case EPrinterType.printer_TX802: v = Visibility.Visible; break;
           }
            Button_Wash.Visibility = v;
            Button_Glue.Visibility = v;
        }

        //--- WebIn_Clicked -------------------------------------------------
        private void WebIn_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, "CMD_CLEAR_ERROR");
            if (RxGlobals.PrintSystem.IsScanning) RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, "CMD_SETUP/CMD_WEBIN");
            else RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, "CMD_PAUSE");
        }

        //--- Clean_Clicked -------------------------------------------------
        private void Clean_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, "CMD_SETUP/CMD_CLEANING");
        }

        //--- Washing_Clicked -------------------------------------------------
        private void Washing_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxMessageBox.YesNo("Washing", "Start Washing the belt?",  MessageBoxImage.Question, false))
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
            if (RxMessageBox.YesNo("Gluing", "Start Gluig?",  MessageBoxImage.Question, false))
                RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, "CMD_SETUP/CMD_GLUE");
        }

        //--- WebIn_IsVisibleChanged ------------------------------
        private void WebIn_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue)
            {
                PrinterStatusChanged(null, null);
            }
        }

        //--- Jog_PreviewMouseDown -------------------------------
        private void Jog_PreviewMouseDown(object sender, MouseButtonEventArgs e) 
        {
            RxButton button = sender as RxButton;
            if (button!=null) 
            {
                Debug.WriteLine("PreviewMouseDown", button.Name);
                button.IsChecked = true;
                RxGlobals.Plc.SetVar(button.Name, 1);
            }
            e.Handled = true;
        }

        //--- Jog_PreviewMouseUp ------------------------------------
        private void Jog_PreviewMouseUp(object sender, MouseButtonEventArgs e) 
        {
            RxButton button = sender as RxButton;
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
