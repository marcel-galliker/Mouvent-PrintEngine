using QRCoder;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows;
using System.Windows.Media.Imaging;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for UserAccount.xaml
    /// </summary>
    public partial class UserAccount : Window, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        public UserAccount()
        {
            InitializeComponent();

            UserRole.DataContext = RxGlobals.User;
            _LicenseChanged();
            RX_DigiPrint.Models.License.OnLicenseChanged += _LicenseChanged;

            try
            {
                UserRole.SelectedValue = RxGlobals.User.UserType;
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
            }

            BluetoothPanel.DataContext = RxGlobals.Bluetooth;
            RxGlobals.Bluetooth.GenerateConnectionId();
            DeviceName.Text = "Device: " + System.Environment.MachineName;
        }

        //--- _LicenseChanged ---------------------------
        void _LicenseChanged()
        {
            UserRole.ItemsSource = new EN_UserTypeList();
        }


        //--- Window_Loaded -------------------------------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            RxGlobals.Screen.PlaceWindow(this);

            try
            {
                RxGlobals.BluetoothLoginWnd = this;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
         
            string code = RxGlobals.Bluetooth.Address + "\n" + RxGlobals.PrinterProperties.Host_Name + "\n" + RxGlobals.Bluetooth.ConnectionId;

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

        private void Confirm_Clicked(object sender, RoutedEventArgs e)
        {
            if (UserRole.SelectedItem != null)
            {
                RxGlobals.User.UserType = (Services.EUserType)UserRole.SelectedValue;
            }

            RxGlobals.License.Update();
            DialogResult = true;
        }

        private void Cancel_Clicked(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }

        private void Bluetooth_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.Bluetooth.SetVisibility();
        }
    }
}
