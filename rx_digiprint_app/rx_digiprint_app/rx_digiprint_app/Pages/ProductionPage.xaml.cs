using DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Xamarin.Forms;
using Xamarin.Forms.Xaml;
using static RX_DigiPrint.Services.RxBtDef;

namespace DigiPrint.Pages
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class ProductionPage : ContentPage
    {
        public ProductionPage()
        {
            InitializeComponent();
            
            Title = AppGlobals.Printer.Name;

            BindingContext = AppGlobals.ProdState;
            Button_Start.Click = Start_Clicked;
            Button_Pause.Click = Pause_Clicked;
            Button_Stop.Click  = Stop_Clicked;
            Button_Abort.Click = Abort_Clicked;
        }

        //--- Start_Clicked ----------------------
        private async void Start_Clicked(object obj)
        {
            SReplyMsg msg = new SReplyMsg(){reply=0};
            if (!AppGlobals.ProdState.AllInkSuppliesOn)
            {
                bool res = await DisplayAlert("Print System", "Some ink supplies are OFF.\nSwitch them ON?", "Yes", "No");
                if (res) msg.reply=1;
            }
            AppGlobals.Bluetooth.SendMsg(RxBtDef.CMD_START_PRINTING, ref msg);
        }
        //--- Pause_Clicked ----------------------
        private void Pause_Clicked(object obj)
        {
            AppGlobals.Bluetooth.SendCommand(RxBtDef.CMD_PAUSE_PRINTING);
        }
        //--- Stop_Clicked ----------------------
        private void Stop_Clicked(object obj)
        {
            AppGlobals.Bluetooth.SendCommand(RxBtDef.CMD_STOP_PRINTING);
        }
        //--- Abort_Clicked ----------------------
        private void Abort_Clicked(object obj)
        {
            AppGlobals.Bluetooth.SendCommand(RxBtDef.CMD_ABORT_PRINTING);
        }
    }
}