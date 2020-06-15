using RX_Common;
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
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.CleafView
{
    /// <summary>
    /// Interaction logic for CLEAF_View.xaml
    /// </summary>
    public partial class CleafView : UserControl
    {
        public CleafView()
        {
            InitializeComponent();
            Button_PrintRelease.Visibility = Visibility.Collapsed;
            CleafOrder.DataContext = RxGlobals.CleafOrder;
            RxGlobals.Timer.TimerFct += Timer;
        }

        //--- Timer -----------------------------------------------------------------
        private void Timer(int no)
        {
            string val = "####";

            if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_cleaf)
            {
                //  RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_GET_VAR, "Application.GUI_00_001_Main"+"\n"+ "STA_PRINT_RELEASE"+"\n");
                RxGlobals.Plc.RequestVar("Application.GUI_00_001_Main" + "\n" + "STA_PRINT_RELEASE" + "\n");
                val = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_PRINT_RELEASE");
            }
            if (val != null && val.Equals("FALSE")) Button_PrintRelease.Visibility = Visibility.Visible;
            else Button_PrintRelease.Visibility = Visibility.Collapsed;
        }

        private void PrintRelease_Clicked(object sender, RoutedEventArgs e)
        {
            if (MvtMessageBox.YesNo("Ready to Print", "Is the machine ready to print?\nNo Splices in the machine?", MessageBoxImage.Question, false))
                RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, "CMD_PRINT_RELEASE");
        }
    }
}
