using DigiPrint.Models;
using RX_DigiPrint.Services;
using System.Threading.Tasks;
using Xamarin.Forms;

namespace DigiPrint.Pages
{
    public partial class MainPage : ContentPage
    {        
        private static BarcodePage  _BarcodePage=null;
        private static bool         _MachinePageActive=false;

        public MainPage()
        {
            InitializeComponent();
            new Task(()=>
                {
                    AppGlobals.Bluetooth.OnLogin        += _OnBluetoothLogin;
                    AppGlobals.Bluetooth.OnDisconnect   += _OnBluetoothDisconnect;
                   _OnBluetoothDisconnect();
                }
            ).Start();
        }

        //--- OnSleep -------------------------------
        public void OnSleep()
        {
            if (_BarcodePage != null) _BarcodePage.OnSleep();
        }

        //--- OnResume -------------------------------
        public void OnResume()
        {
            if (_BarcodePage != null) _BarcodePage.OnResume();
        }

        //--- _OnBluetoothLogin -----------------------
        private void _OnBluetoothLogin()
        {
            if (_BarcodePage!=null) 
            {
                Device.BeginInvokeOnMainThread(()=>
                {
                    Navigation.PopModalAsync();
                 // Navigation.PushModalAsync(new MachinePage ());

                    Navigation.PushModalAsync(new RxTabbedPage());
                    _MachinePageActive =true;
                }
                );
            }
            _BarcodePage=null;
        }

        //--- _OnBluetoothDisconnect ------------------------
        private void _OnBluetoothDisconnect()
        {
            if (_BarcodePage==null) 
            {
                Device.BeginInvokeOnMainThread(()=>
                {   
                    if (_MachinePageActive) Navigation.PopModalAsync();
                    _MachinePageActive = false;
                    _BarcodePage = new BarcodePage();
                    Navigation.PushModalAsync(_BarcodePage);
                });
            }
        }
    }
}
