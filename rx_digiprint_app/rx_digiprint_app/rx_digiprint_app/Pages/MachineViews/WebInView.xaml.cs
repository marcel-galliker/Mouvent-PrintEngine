using DigiPrint.Models;
using System.Threading.Tasks;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;
using static RX_DigiPrint.Services.RxBtDef;

namespace DigiPrint.Pages.MachineViews
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class WebInView : ContentView
    {
        public WebInView()
        {
            InitializeComponent();

            GoFwd.Click     = GoFwd_Clicked;
            GoFwd.MouseUp   = GoFwd_MouseUp;
            GoBwd.Click     = GoBwd_Clicked;
            GoBwd.MouseUp   = GoBwd_MouseUp;
        }

        //--- GoFwd_Clicked --------------------------------
        private async void GoFwd_Clicked(object sender)
        {
            AppGlobals.Vibrate(50);     
            await Task.Delay(300);
            if (GoFwd.IsPressed) AppGlobals.Bluetooth.SendCommand(BT_CMD_JOG_FWD_START);
        }

        //--- GoFwd_MouseUp ----------------------
        private void GoFwd_MouseUp(object sender)
        {
            if (GoFwd.IsPressed)AppGlobals.Bluetooth.SendCommand(BT_CMD_JOG_FWD_STOP);
        }

        //--- GoBwd_Clicked --------------------------------
        private async void GoBwd_Clicked(object sender)
        {
            AppGlobals.Vibrate(100);
            await Task.Delay(500);
            if (GoBwd.IsPressed) AppGlobals.Bluetooth.SendCommand(BT_CMD_JOG_BWD_START);
        }

        //--- GoBwd_MouseUp ----------------------
        private void GoBwd_MouseUp(object sender)
        {
            if (GoBwd.IsPressed) AppGlobals.Bluetooth.SendCommand(BT_CMD_JOG_BWD_STOP);
        }
    }
}