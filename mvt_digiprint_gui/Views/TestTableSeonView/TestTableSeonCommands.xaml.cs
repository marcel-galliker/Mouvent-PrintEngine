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

namespace RX_DigiPrint.Views.TestTableSeonView
{
    /// <summary>
    /// Interaction logic for TestTableSeonCommands.xaml
    /// </summary>
    public partial class TestTableSeonCommands : UserControl
    {
        public TestTableSeonCommands()
        {
            InitializeComponent();
        }

        //--- Stop_Clicked -------------------------------------------------
        private void Stop_Clicked(object sender, RoutedEventArgs e)
        {
            Button_Stop.IsChecked = false;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TT_STOP);
        }

        //--- StartRef_Clicked -------------------------------------------------
        private void StartRef_Clicked(object sender, RoutedEventArgs e)
        {
            Button_Ref.IsChecked = false;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_LIFT_REFERENCE);
        }

        //--- ScanRight_Clicked -------------------------------------------------
        private void ScanFront_Clicked(object sender, RoutedEventArgs e)
        {
            Button_StF.IsChecked = false;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TT_SCAN_RIGHT);
        }

        //--- ScanLeft_Clicked -------------------------------------------------
        private void ScanBack_Clicked(object sender, RoutedEventArgs e)
        {
            Button_StB.IsChecked = false;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TT_SCAN_LEFT);
        }

        //--- ScanWaste_Clicked -------------------------------------------------
        private void ScanWaste_Clicked(object sender, RoutedEventArgs e)
        {
            Button_StW.IsChecked = false;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TT_SCAN_WASTE);
        }

        //--- ScanJet_Clicked -------------------------------------------------
        private void ScanJet_Clicked(object sender, RoutedEventArgs e)
        {
            Button_StJ.IsChecked = false;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TT_SCAN_TRAY);
        }

        //--- ScanPurge_Clicked -------------------------------------------------
        private void ScanPurge_Clicked(object sender, RoutedEventArgs e)
        {
            Button_StP.IsChecked = false;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TT_MOVE_PURGE);
        }

        private void ScanTable_Clicked(object sender, RoutedEventArgs e)
        {
            Button_StT.IsChecked = false;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TT_MOVE_ADJUST);
        }

        private void Up_Clicked(object sender, RoutedEventArgs e)
        {
            Button_Up.IsChecked = false;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_LIFT_UP_POS);
        }
    }
}
