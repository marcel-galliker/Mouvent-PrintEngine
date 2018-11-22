using DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Xamarin.Forms;
using Xamarin.Forms.Xaml;
using static RX_DigiPrint.Services.RxBtDef;

namespace DigiPrint.Pages.PrintSystemView
{
	[XamlCompilation(XamlCompilationOptions.Compile)]
	public partial class HeadDetailsView : ContentView
    {
        public HeadDetailsView()
		{
			InitializeComponent ();

            var tgr = new TapGestureRecognizer 
            { 
                NumberOfTapsRequired = 1,
                TappedCallback = (sender, args) => 
                {
                    Mode.IsVisible=!Mode.IsVisible;
                },
            };

            MainGrid.GestureRecognizers.Add(tgr);
        }

        //--- BindingContextChanged -----------------------------
        private new void BindingContextChanged(object sender, EventArgs e)
        {
            PrintHead head = BindingContext as PrintHead;
            Mode.IsVisible=false;
            if (head!=null) head.PropertyChanged += Head_PropertyChanged;
        }

        //--- Head_PropertyChanged ---------------------------
        private void Head_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("IsSelected"))
            {
                MainGrid.BackgroundColor = (BindingContext as PrintHead).IsSelected ? Color.LightBlue : Color.WhiteSmoke;
            }
        }

        private void _mode_clicked(EFluidCtrlMode ctrlMode)
        {
            PrintHead head = BindingContext as PrintHead;
            SBtFluidCtrlCmd msg = new SBtFluidCtrlCmd();
            msg.no          = head.no;
            msg.ctrlMode    = ctrlMode; 
            AppGlobals.Bluetooth.SendMsg(CMD_FLUID_CTRL_MODE, ref msg);
            Mode.IsVisible=false;
        }
        private void Off_Clicked(object sender, EventArgs e){ _mode_clicked(EFluidCtrlMode.ctrl_off);}
        private void Print_Clicked(object sender, EventArgs e){ _mode_clicked(EFluidCtrlMode.ctrl_print);}
        private void Flush_Clicked(object sender, EventArgs e){ _mode_clicked(EFluidCtrlMode.ctrl_flush);}
        private void PurgeS_Clicked(object sender, EventArgs e){ _mode_clicked(EFluidCtrlMode.ctrl_purge_soft);}
        private void Purge_Clicked(object sender, EventArgs e){ _mode_clicked(EFluidCtrlMode.ctrl_purge);}
        private void PurgeH_Clicked(object sender, EventArgs e){ _mode_clicked(EFluidCtrlMode.ctrl_purge_hard);}
    }
}