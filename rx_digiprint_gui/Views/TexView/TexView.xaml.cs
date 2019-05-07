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
using System.Windows.Navigation;
using System.Windows.Shapes;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.UserControls;
using System.Diagnostics;
using RX_Common;

namespace RX_DigiPrint.Views.TexView
{
    public partial class TexView : UserControl
    {
        RxLanguage language = new RxLanguage();

        public TexView()
        {
            InitializeComponent();
            this.DataContext = this;
            RB_Main.IsChecked=true;
            PlcNotConnected.DataContext = RxGlobals.Plc;
        //    Consumables.CalibrationView = Calibration;
        }
        
        //--- Save_Clicked --------------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_SAVE_PAR);
        }

        //--- Reload_Clicked --------------------------------------------------
        private void Reload_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_LOAD_PAR);
        }
    }
}
