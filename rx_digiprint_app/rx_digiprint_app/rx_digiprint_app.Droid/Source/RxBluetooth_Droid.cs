using Android.Bluetooth;
using Android.Content;
using Java.Lang.Reflect;
using Java.Util;
using RX_DigiPrint.Services;
using System;
using System.Threading;

// https://blog.xamarin.com/barcode-scanning-made-easy-with-zxing-net-for-xamarin-forms/

namespace DigiPrint.Droid
{
    public class RxBluetooth_Droid : RxBluetooth
    {
        private BluetoothSocket _Socket;
        private Action<Intent> _StartActivity;

        private static UUID MouventBlueToothGuid = UUID.FromString(RxBtDef.MouventBlueToothGuid);    

        //--- constructor ------------------------
        public RxBluetooth_Droid(Action<Intent> startActivity)
        {
            _StartActivity = startActivity;
            base.Connect += Connect;
        }

        //--- Connect -----------------------------------------------------
        public new EBtResult Connect(string deviceAddress, string pin)
        {
            try
            {
                if (BluetoothAdapter.DefaultAdapter == null) return EBtResult.result_error;
                if (!BluetoothAdapter.DefaultAdapter.IsEnabled) return EBtResult.result_disabled;

                BluetoothDevice device = BluetoothAdapter.DefaultAdapter.GetRemoteDevice(deviceAddress);
                
                if (device.BondState!=Bond.Bonded) 
                {
                    Boolean ok=device.CreateBond();

                    int i=0;
                    do 
					{
                        i++;
                        Console.WriteLine("BondState[{0}]={1}", i, device.BondState.ToString());
                        Thread.Sleep(100);
                        if (i>100) return EBtResult.result_not_bound;
					} while (device.BondState!=Bond.Bonded);
                }

                _Socket = device.CreateRfcommSocketToServiceRecord(MouventBlueToothGuid);
                _Socket.Connect();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error {0}", ex.Message);
                return EBtResult.result_error;
            }
            StreamOut = _Socket.OutputStream;
            StreamIn = _Socket.InputStream;
            Thread.Sleep(100);
            return EBtResult.result_ok;
        }

        //--- Deconnect -----------------------------
        public new void Deconnect()
        {
            base.Deconnect();
            _Socket.Close();
            _Socket = null;
        }
    }
}
