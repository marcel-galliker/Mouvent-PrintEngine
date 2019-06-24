using Android.Content;
using DigiPrint.Common;
using DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

using Xamarin.Forms;
using Xamarin.Forms.Xaml;
using ZXing;
using ZXing.Net.Mobile.Forms;
using static RX_DigiPrint.Services.RxBluetooth;
using static RX_DigiPrint.Services.RxBtDef;

namespace DigiPrint.Pages
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class BarcodePage : ContentPage
    {
        private bool                _Demo = false;

        private ZXingScannerView    _bcScanner;
        private Task                _animationTask;

        //--- BarcodePage --------------------------
        public BarcodePage()
        {
            InitializeComponent();

            LicenseDlg.BindingContext = AppGlobals.License;
            OnResume();

            _animationTask = new Task(_Animation);
            _animationTask.Start();

            Mail.Click     = Mail_Clicked;
            Demo.IsVisible = _Demo;
        }

        //--- OnSleep ----------------------------
        public void OnSleep()
        {
        }
        //--- OnResume -------------------------------
        public void OnResume()
        {
            _bcScanner = new ZXingScannerView();
            _bcScanner.OnScanResult += OnScanResult;
            _bcScanner.IsTorchOn = true;
            _bcScanner.IsScanning = true;
            _bcScanner.AutoFocus();
            if (ScanView.Children.Count == 0) ScanView.Children.Add(_bcScanner);
            else                              ScanView.Children[0] = _bcScanner;
        }

        //--- _Animation -------------------------
        async void _Animation()
        {
            while(true)
            {
                await Cursor.TranslateTo(0, ActiveRegion.Height, 1000);
                await Cursor.TranslateTo(0,   0,    1000);
                if (_Demo)
                {
                    AppGlobals.Printer.State = RxBtDef.EBTState.state_webin;
                    AppGlobals.Bluetooth.OnLogin();
                    break;
                }
            }
        }

        //--- OnScanResult ----------------------------------
        private void OnScanResult(Result result)
        {
            string[] line = result.Text.Split('\n');
            // [0]: Device Name
            // [1]: Machine Name
            // [2]: Connection ID
            AppGlobals.Printer.Name  = line[1];
            if (line.Length>2 && !AppGlobals.Bluetooth.IsConnected && AppGlobals.Bluetooth.Connect!=null) 
            {
            //    EBtResult res = AppGlobals.Bluetooth.Connect(line[0]);
			//	Task.Delay(1000);
                switch (AppGlobals.Bluetooth.Connect(line[0]))
                {
                    case EBtResult.result_ok:
                        if (AppGlobals.Bluetooth.IsConnected)
                        {
                            SLogInMsg msg = new SLogInMsg
                            {
                                connectionId = line[2],
                                userName = AppGlobals.DeviceId.UserName,
                                license = AppGlobals.License.Code
                            };
                            AppGlobals.Bluetooth.SendMsg(BT_CMD_LOGIN, ref msg);
                        }
                        else RxError.Error(this, "Can not start BLUETOOTH connection. Not Connected.");
                        break;

                    case EBtResult.result_disabled:     RxError.Error(this, "Check BLUETOOTH is enabled");break;
                    case EBtResult.result_not_bound:    RxError.Error(this, "Can not start BLUETOOTH connection. Check that device is paired."); break;
                    case EBtResult.result_error:        RxError.Error(this, "BLUETOOTH. Exception."); break;
                }
            }
        }

        //--- SizeChanged ----------------------------------
        private void ScanView_SizeChanged(object sender, EventArgs e)
        {
            double size=ScanView.Width;
            if (ScanView.Height<size) size=ScanView.Height;
            size = size*0.6;
            ActiveRegion.HeightRequest = size;
            ActiveRegion.WidthRequest  = size;
        }

        //--- Mail_Clicked ---------------------------------
        private void Mail_Clicked(object sender)
        {
            RxMail mail = new RxMail
            {
                To = "license@mouvent.com",
                Subject = "License Request",
                Text = "Licence Request for " + AppGlobals.DeviceId.UserName + "\n\n" + AppGlobals.DeviceId.ToString(),
                AttachmentContent = AppGlobals.Crypt.Encrypt(AppGlobals.DeviceId.Info(), RxBtDef.InfoPwd)
            };
            AppGlobals.MailServer.Send(mail);
        }
    }
}