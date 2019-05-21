using QRCoder;
using RX_DigiPrint.Models;
using System;
using System.Collections.Generic;
using System.Drawing;
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
    public partial class BluetoothWindow : Window
    {
        // https://github.com/codebude/QRCoder

        //--- Constructor ----------------------------------
        public BluetoothWindow()
        {
            InitializeComponent();
            DataContext = RxGlobals.Bluetooth;
            RxGlobals.Bluetooth.GenerateConnectionId();
            DeviceName.Text = "Device: "+System.Environment.MachineName;
        }

        //--- Window_Loaded -------------------------------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            RxGlobals.BluetoothLoginWnd = this;

            string code = RxGlobals.Bluetooth.Address+"\n"+RxGlobals.PrinterProperties.Host_Name+"\n"+RxGlobals.Bluetooth.ConnectionId;

            QRCodeGenerator qrGenerator = new QRCodeGenerator();
            QRCodeData qrCodeData = qrGenerator.CreateQrCode(code, QRCodeGenerator.ECCLevel.Q);
            QRCode qrCode = new QRCode(qrCodeData);
            Bitmap qrCodeImage = qrCode.GetGraphic(20);

            Barcode.Source = System.Windows.Interop.Imaging.CreateBitmapSourceFromHBitmap(
                          qrCodeImage.GetHbitmap(),
                          IntPtr.Zero,
                          Int32Rect.Empty,
                          BitmapSizeOptions.FromEmptyOptions());
            RxGlobals.Bluetooth.DeviceVisible();
        }

        //--- Window_Closing -------------------------------------
        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
             RxGlobals.BluetoothLoginWnd = null;
        }

        //--- Close_Clicked -------------------------
        private void Close_Clicked(object sender, RoutedEventArgs e)
        {
            Close();
        }
    }
}
