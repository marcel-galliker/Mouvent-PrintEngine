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
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Views.Settings;

namespace RX_DigiPrint.Views.SupervisorsView
{
    /// <summary>
    /// Class managing the QR code displaying view
    /// </summary>
    public partial class QRCodeView : Window
    {
        public QRCodeView(String username)
        {
            InitializeComponent();
            MvtUserLevelManager ulm = new MvtUserLevelManager(SettingsDlg.GetVersion(), RxGlobals.PrinterProperties.Host_Name);
            MvtTOTP codes = ulm.GetSupervisorCode(username);
            SupervisorCodes.NavigateToString(String.Format("<html><body><div style=\"text-align:center\"><img src='{0}' width=\"300\" height=\"300\"/><br>{1}</div></body></html>", codes.GetQRCode(), codes.GetManualCode()));
        }

        private void Close_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }
    }
}
