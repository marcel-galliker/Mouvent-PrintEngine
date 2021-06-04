using RX_DigiPrint.Models;
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
    public partial class PowerOff : Window
    {
        public PowerOff()
        {
            InitializeComponent();
        }

        //--- TurnOffHeads_Clicked -------------------------------------------------
        private void TurnOffHeads_Clicked(object sender, RoutedEventArgs e)
        {
            TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd() { no = -1, ctrlMode = EFluidCtrlMode.ctrl_shutdown };
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ENCODER_UV_OFF);
            Close();
        }

        //--- ShutDown_Clicked -------------------------------------------------
        private void ShutDown_Clicked(object sender, RoutedEventArgs e)
        {
            Close();
            TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd() { no = -1, ctrlMode = EFluidCtrlMode.ctrl_shutdown };
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ENCODER_UV_OFF);
            ShuttingDown sdn = new ShuttingDown();
            sdn.ShowDialog();
            //ShutDown sdn = new ShutDown();
            //sdn.ShowDialog();
        }
    }
}
