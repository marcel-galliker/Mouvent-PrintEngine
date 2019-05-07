using DigiPrint.Common;
using DigiPrint.Pages;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xamarin.Forms;

namespace DigiPrint.Models
{
    public class AppGlobals
    {
        public static RxBluetooth                       Bluetooth;
        public static DeviceId                          DeviceId;
        public static ObservableCollection<RxEvents>    Events  = new ObservableCollection<RxEvents>();
        public static License                           License = new License();
        public static Printer                           Printer = new Printer();

        public static ObservableCollection<PrintHead>   PrintHeads = new ObservableCollection<PrintHead>();
        public static Action                            ActiveHeadChanged;

        public static ObservableCollection<StepperMotor> Steppers = new ObservableCollection<StepperMotor>();

        public static ImageSource                       Preview;

        public static IRxMailServer                     MailServer;
        public static IRxCrypt                          Crypt;
        public static Action<int>                       Vibrate;

        public static Func<string, System.IO.Stream>    FileOpen;
        public static Func<string, System.IO.Stream>    FileCreate;
        public static string                            DirDownloads;

        public static RxTimer                           Timer_200ms = new RxTimer(200);
        public static RxProdState                       ProdState = new RxProdState();
    }
}
