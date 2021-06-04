using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Resources.Language;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for ShuttingDown.xaml
    /// </summary>
    public partial class ShuttingDown : Window
    {
        private int _headStatNb = 0;

        public ShuttingDown()
        {
            InitializeComponent();

            foreach (InkSupply inkSupply in RxGlobals.InkSupply.List)
            {
                inkSupply.PropertyChanged += InkSupply_PropertyChanged;
            }
            RxGlobals.UvLamp.PropertyChanged += UvLamp_PropertyChanged;
            /*foreach (HeadStat headStat in RxGlobals.HeadStat.List)
            {
                headStat.PropertyChanged += HeadStat_PropertyChanged;
            }*/

            // Progress bar initialization
            ProgressBar.Minimum = 0;
            ProgressBar.Maximum = 1;
            foreach (InkSupply inkSupply in RxGlobals.InkSupply.List)
            {
                if (inkSupply.Connected) ProgressBar.Maximum++;
            }
            foreach (HeadStat headStat in RxGlobals.HeadStat.List)
            {
                if (headStat.Connected) _headStatNb++;
            }
            ProgressBar.Maximum += _headStatNb;

            // Switch Off UV lamp
            if (RxGlobals.UvLamp.On || RxGlobals.UvLamp.Ready)
            {
                if ((RxGlobals.UvLamp.ActionStr == null || !RxGlobals.UvLamp.ActionStr.Equals("OFF")) && MvtMessageBox.YesNo(RX_DigiPrint.Resources.Language.Resources.UvDryer, RX_DigiPrint.Resources.Language.Resources.SwitchOff, MessageBoxImage.Question, false))
                {
                    RxGlobals.UvLamp.SwitchOff();
                }
            }
        }

        //--- InkSupply_PropertyChanged ---------------------------------------                                                                           
        private void InkSupply_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            _ShutdownSystem();
        }

        //--- UvLamp_PropertyChanged ---------------------------------------                                                                           
        private void UvLamp_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            _ShutdownSystem();
        }

        //--- HeadStat_PropertyChanged ---------------------------------------                                                                           
        private void HeadStat_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            int headsFlushedNb = 0;
            bool headsFlushed = true;
            foreach (HeadStat headStat in RxGlobals.HeadStat.List)
            {
                if (headStat.Connected)
                {
                    if (headStat.CtrlMode == EFluidCtrlMode.ctrl_flush_done) headsFlushedNb++;
                    else headsFlushed = false;
                }
            }
            ProgressBar.Value = headsFlushedNb;
            if (headsFlushed)
            {
                // Shutdown heads
                Status.Text = RX_DigiPrint.Resources.Language.Resources.HeadsShuttingDown;
                TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd() { no = -1, ctrlMode = EFluidCtrlMode.ctrl_shutdown };
                RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
                RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ENCODER_UV_OFF);
            }
            else
                Status.Text = RX_DigiPrint.Resources.Language.Resources.HeadsFlushing;
        }

        //--- _ShutdownSystem -------------------------------------------------
        private void _ShutdownSystem()
        {
            int inkSupplyOffNb = 0;
            bool inkSupplyOff = true;
            foreach (InkSupply inkSupply in RxGlobals.InkSupply.List)
            {
                if (inkSupply.Connected)
                {
                    if (inkSupply.CtrlMode == EFluidCtrlMode.ctrl_off) inkSupplyOffNb++;
                    else inkSupplyOff = false;
                }
            }
            if (inkSupplyOffNb > 0) ProgressBar.Value = _headStatNb + inkSupplyOffNb;
            if (inkSupplyOff)
            {
                Status.Text = RX_DigiPrint.Resources.Language.Resources.UvLampShuttingDown;
                if (!RxGlobals.UvLamp.On && !RxGlobals.UvLamp.Ready && !RxGlobals.UvLamp.Busy)
                {
                    // Shutdown server and GUI
                    ProgressBar.Value = ProgressBar.Maximum;
                    RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SHUTDOWN_MAIN);
                    Status.Text = RX_DigiPrint.Resources.Language.Resources.MainShuttingDown;
                    Rx.StartProcess("shutdown", "/s /t 2");
                }
            }
            else
                Status.Text = RX_DigiPrint.Resources.Language.Resources.HeadsShuttingDown;
        }

        //--- Cancel_Clicked -------------------------------------------------
        private void Cancel_Clicked(object sender, RoutedEventArgs e)
        {
            foreach (InkSupply inkSupply in RxGlobals.InkSupply.List)
            {
                inkSupply.PropertyChanged -= InkSupply_PropertyChanged;
            }
            RxGlobals.UvLamp.PropertyChanged -= UvLamp_PropertyChanged;
            /*foreach (HeadStat headStat in RxGlobals.HeadStat.List)
            {
                headStat.PropertyChanged -= HeadStat_PropertyChanged;
            }*/
            Close();
        }
    }
}
