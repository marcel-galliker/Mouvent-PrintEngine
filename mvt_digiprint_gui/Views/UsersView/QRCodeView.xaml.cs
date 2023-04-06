using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Views.Settings;
using System;
using System.Windows;

namespace RX_DigiPrint.Views.UsersView
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
            MvtTOTP codes = ulm.GetUserCode(username);
            
            QRCode.Source = codes.QRImgSrc();
            ManualCode.Text = codes.GetManualCode();
        }

        private void Close_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }
    }
}
