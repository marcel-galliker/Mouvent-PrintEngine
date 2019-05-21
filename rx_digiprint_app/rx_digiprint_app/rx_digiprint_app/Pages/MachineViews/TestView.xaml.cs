using DigiPrint.Models;
using System;

using Xamarin.Forms;
using Xamarin.Forms.Xaml;
using static RX_DigiPrint.Services.RxBtDef;

namespace DigiPrint.Pages.MachineViews
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class TestView : ContentView
    {
        public TestView()
        {
            InitializeComponent();

            Button3.Click   = Clicked;
            Button3.MouseUp = MouseUp;
            Button4.Click   = Button4_Clicked;
        }
        private void Clicked(object sender)
        {
            SReplyMsg msg = new SReplyMsg{reply = 1};
            AppGlobals.Bluetooth.SendMsg(BT_CMD_BUTTON_3, ref msg);
        }
        private void MouseUp(object sender)
        {
            SReplyMsg msg = new SReplyMsg{reply = 0};
            AppGlobals.Bluetooth.SendMsg(BT_CMD_BUTTON_3, ref msg);
        }

        //--- Button1_Clicked --------------------------------------
        private void Button1_Clicked(object sender, EventArgs e)
        {
            SReplyMsg msg = new SReplyMsg{reply = 1};
            AppGlobals.Bluetooth.SendMsg(BT_CMD_BUTTON_1, ref msg);
//            DisplayAlert ("Test", "TEST", "OK");
        }

        //--- Button2_Clicked ---------------------------------
        private void Button2_Clicked(object sender, EventArgs e)
        {
            SReplyMsg msg = new SReplyMsg{reply = 1};
            AppGlobals.Bluetooth.SendMsg(BT_CMD_BUTTON_2, ref msg);
        }

        //--- Button3_Clicked --------------------------------
        private void Button3_Clicked(object sender, EventArgs e)
        {
            SReplyMsg msg = new SReplyMsg{reply = 1};
            AppGlobals.Bluetooth.SendMsg(BT_CMD_BUTTON_3, ref msg);
        }


        //--- Button4_Clicked ----------------------------
        private void Button4_Clicked(object sender)
        {
            SReplyMsg msg = new SReplyMsg{reply = 1};
            AppGlobals.Bluetooth.SendMsg(BT_CMD_BUTTON_4, ref msg);
        }

        private void Button1_Unfocused(object sender, FocusEventArgs e)
        {

        }
    }
}