using System;
using DigiPrint.Models;

using Xamarin.Forms;
using Xamarin.Forms.Xaml;
using RX_DigiPrint.Services;

namespace DigiPrint.Pages
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class EventsPage : ContentPage
    {
        public EventsPage()
        {
            InitializeComponent();

            //  BindingContext  = this;
            Events.ItemsSource  = AppGlobals.Events;
            Confirm.Click       = Confirm_Clicked; 
        }

        //--- Confirm_Clicked ---------------------------------
        private void Confirm_Clicked(object obj)
        {
            AppGlobals.Bluetooth.SendCommand(RxBtDef.BT_CMD_EVT_CONFIRM);
        }
    }
}