
using Android;
using Android.App;
using Android.Content;
using Android.Content.PM;
using Android.OS;
using Android.Provider;
using Android.Support.V4.App;
using Android.Util;
using DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.IO;
using Xamarin.Forms.Xaml;
using ImageCircle.Forms.Plugin.Droid;

[assembly: XamlCompilation(XamlCompilationOptions.Compile)]

namespace DigiPrint.Droid
{

    [Activity(  Label = "DigiPrint", 
                Icon = "@drawable/icon", 
                Theme = "@style/MainTheme",
                ScreenOrientation = ScreenOrientation.Portrait,
                ConfigurationChanges = ConfigChanges.ScreenSize | ConfigChanges.Orientation)]
    public class MainActivity : global::Xamarin.Forms.Platform.Android.FormsAppCompatActivity
    {
        PowerManager.WakeLock _wakeLock;

        //--- OnCreate ---------------------------------------------
        protected override void OnCreate(Bundle bundle)
        {
            _permissions();

            ZXing.Net.Mobile.Forms.Android.Platform.Init();
            global::Xamarin.Forms.Forms.Init(this, bundle);
            
            ImageCircleRenderer.Init();

            AppGlobals.Bluetooth = new RxBluetooth_Droid(StartActivity);
            TabLayoutResource = Resource.Layout.Tabbar;
            ToolbarResource = Resource.Layout.Toolbar;

            base.OnCreate(bundle);

            LoadApplication(new App());

            //-- to allow emails with file attachments ---
            StrictMode.VmPolicy.Builder builder = new StrictMode.VmPolicy.Builder();
            StrictMode.SetVmPolicy(builder.Build());

            //--- platform specific functions -----------
            AppGlobals.DeviceId = new DeviceId()
            {
                Manufacturer = Build.Manufacturer,
                Model = Build.Model,
                Serial = Build.Serial,
                UserName = _userName(),
            };
            AppGlobals.MailServer     = new RxMailserver_Driod(){StartActivity = StartActivity};
            AppGlobals.Crypt          = new RxEncypt();
            AppGlobals.Vibrate        = _Vibrate;
            AppGlobals.FileOpen       = _FileOpen;
            AppGlobals.FileCreate     = _FileCreate;
            AppGlobals.DirDownloads   = Android.OS.Environment.GetExternalStoragePublicDirectory(Android.OS.Environment.DirectoryDownloads).Path;
            PowerManager _powerManager = (PowerManager)GetSystemService(PowerService);
            _wakeLock = _powerManager.NewWakeLock(WakeLockFlags.Full, "no sleep");
        }

        //--- _readLicense ------------------------------------
        private void _readLicense()
        {
            try
            {
                //--- in downloads the files are numerated --- select the ome with the hoghest number, delete the rest ---
                string filepath = Path.Combine(Android.OS.Environment.GetExternalStoragePublicDirectory(Android.OS.Environment.DirectoryDocuments).Path, RxBtDef.LicFileName);
                string[] files  = System.IO.Directory.GetFiles(Android.OS.Environment.GetExternalStoragePublicDirectory(Android.OS.Environment.DirectoryDocuments).Path, RxBtDef.LicFileName+"*");
                if (files.Length>1)
                {
                    int i;
                    for (i=0; i<files.Length-1; i++)
                        System.IO.File.Delete(files[i]);
                    System.IO.File.Move(files[i], filepath);
                }
                AppGlobals.License.Code = System.IO.File.ReadAllText(filepath);
                AppGlobals.License.PlainCode = AppGlobals.Crypt.Decrypt(AppGlobals.License.Code, RxBtDef.LicPwd);
                if (AppGlobals.License.Valid && !AppGlobals.License.Expired)
                {
                    try{ System.IO.File.Delete(Path.Combine(Android.OS.Environment.GetExternalStoragePublicDirectory(Android.OS.Environment.DirectoryDocuments).Path, "requestinfo.txt"));}
                    catch{};
                }
            }
            catch
            {
                AppGlobals.License.Code = null;
            }

        }

        //--- _FileOpen -------------------------------------
        private Stream _FileOpen(string path)
        {
            return new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
        }

        //--- _FileCreate -------------------------------------
        private Stream _FileCreate(string path)
        {
            return new FileStream(path, FileMode.Create, FileAccess.Write, FileShare.ReadWrite);
        }

        //--- permissions -----------------------------------------
        private void _permissions()
        {
            string[] permissions =
            {
                Manifest.Permission.Camera,
                Manifest.Permission.ReadExternalStorage,
                Manifest.Permission.WriteExternalStorage,
                Manifest.Permission.Bluetooth,
                Manifest.Permission.BluetoothAdmin,
                Manifest.Permission.ReadContacts,
                Manifest.Permission.ReadUserDictionary,
                Manifest.Permission.WriteContacts,
                Manifest.Permission.ReadProfile,
                Manifest.Permission.Vibrate,
                Manifest.Permission.WriteProfile,
                Manifest.Permission.WakeLock
            };
            ActivityCompat.RequestPermissions(this, permissions, 0);
        }

        //--- OnPause ------------------------------------
        protected override void OnPause()
        {
            base.OnPause();
            _wakeLock.Release();
        }

        //--- OnResume -----------------------------------
        protected override void OnResume()
        {
            base.OnResume();
            _wakeLock.Acquire();
            _readLicense();
        }

        //--- _userName -----------------------------------
        private string _userName()
        {
            string[] projection = {ContactsContract.Profile.InterfaceConsts.DisplayName};

            try
            {
                var cursor = ContentResolver.Query(ContactsContract.Profile.ContentUri, projection, null, null, null);
                if (cursor.MoveToFirst ()) 
                {
                    Log.Info("DigiPrint", "_userName 2");
                    string user = cursor.GetString(cursor.GetColumnIndex(projection[0]));
                    return user;
                }
                else return "Somebody";
            }
            catch
            {
            }
            return null;
        }

        //--- OnRequestPermissionsResult --------------------------------
        public override void OnRequestPermissionsResult(int requestCode, string[] permissions, Permission[] grantResults)
        {
            global::ZXing.Net.Mobile.Android.PermissionsHandler.OnRequestPermissionsResult (requestCode, permissions, grantResults);
            base.OnRequestPermissionsResult(requestCode, permissions, grantResults);
        }

        //--- Vibrate ----------------------------------
        private Vibrator _vibrator;

        public void _Vibrate(int time)
        {
            if (_vibrator==null) _vibrator =  (Vibrator) GetSystemService(Context.VibratorService);
            _vibrator.Vibrate(time);
        }
    }
}

