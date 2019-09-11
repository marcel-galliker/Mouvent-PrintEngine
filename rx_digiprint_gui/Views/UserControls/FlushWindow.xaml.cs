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
    public partial class FlushWindow : Window
    {
        public FlushWindow()
        {
            InitializeComponent();
        }

        //--- Night_Clicked -------------------------------------------------
        private void Night_Clicked(object sender, RoutedEventArgs e)
        {
            TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd(){no=-1, ctrlMode = EFluidCtrlMode.ctrl_flush_night};
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
            Close();
        }
 
        //--- Weekend_Clicked -------------------------------------------------
        private void Weekend_Clicked(object sender, RoutedEventArgs e)
        {
            TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd(){no=-1, ctrlMode = EFluidCtrlMode.ctrl_flush_weekend};
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
            Close();
        }

        //--- Week_Clicked -------------------------------------------------
        private void Week_Clicked(object sender, RoutedEventArgs e)
        {
            TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd(){no=-1, ctrlMode = EFluidCtrlMode.ctrl_flush_week};
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
            Close();
        }

        //--- Yes_Clicked -------------------------------------------------
        private void Yes_Clicked(object sender, RoutedEventArgs e)
        {
            TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd(){no=-1, ctrlMode = EFluidCtrlMode.ctrl_off};
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ENCODER_UV_OFF);
            Close();
        }

        //--- No_Clicked -------------------------------------------------
        private void No_Clicked(object sender, RoutedEventArgs e)
        {
            Close();
        }
        
    }
}
