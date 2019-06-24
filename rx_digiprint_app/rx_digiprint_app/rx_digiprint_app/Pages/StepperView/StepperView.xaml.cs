using DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace DigiPrint.Pages.StepperView
{
	[XamlCompilation(XamlCompilationOptions.Compile)]
	public partial class StepperView : ContentView
	{
		public StepperView ()
		{
			InitializeComponent ();
            AppGlobals.Steppers.CollectionChanged += Steppers_CollectionChanged;
            Steppers_CollectionChanged(null, null);

            AppGlobals.Timer_200ms.Add_Action(_refresh);
		}
        //--- _refresh -------------------------------------------
        private void _refresh()
        {
            if (IsVisible) AppGlobals.Bluetooth.SendCommand(RxBtDef.BT_CMD_STEPPER_STATE);
        }
        //--- Steppers_CollectionChanged ---------------------------
        private void Steppers_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            int i;
            for (i=0; i<AppGlobals.Steppers.Count; i++)
            {
                switch(i)
                {
                    case 0: MotorFL.BindingContext = AppGlobals.Steppers[i]; break;
                    case 1: MotorBL.BindingContext = AppGlobals.Steppers[i]; break;
                    case 2: MotorBR.BindingContext = AppGlobals.Steppers[i]; break;
                    case 3: MotorFR.BindingContext = AppGlobals.Steppers[i]; break;
                }
            }
        }

        private void Stop_Clicked   (object sender, EventArgs e){ AppGlobals.Bluetooth.SendCommand(RxBtDef.BT_CMD_STEPPER_STOP); }
        private void Ref_Clicked    (object sender, EventArgs e){ AppGlobals.Bluetooth.SendCommand(RxBtDef.BT_CMD_STEPPER_REF); }
        private void Up_Clicked     (object sender, EventArgs e){ AppGlobals.Bluetooth.SendCommand(RxBtDef.BT_CMD_STEPPER_UP); }
        private void Print_Clicked  (object sender, EventArgs e){ AppGlobals.Bluetooth.SendCommand(RxBtDef.BT_CMD_STEPPER_PRINT); }
        private void Cap_Clicked    (object sender, EventArgs e){ AppGlobals.Bluetooth.SendCommand(RxBtDef.BT_CMD_STEPPER_CAP); }

    }
}