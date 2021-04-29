using System;
using System.Collections.Generic;
using System.IO;
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
            
            QRCode.Source = codes.QRImgSrc();
            ManualCode.Text = codes.GetManualCode();
        }

        private void Close_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }
    }
}
