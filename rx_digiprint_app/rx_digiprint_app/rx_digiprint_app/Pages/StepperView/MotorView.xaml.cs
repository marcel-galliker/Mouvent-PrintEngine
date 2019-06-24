using DigiPrint.Common;
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

namespace DigiPrint.Pages.StepperView
{
	[XamlCompilation(XamlCompilationOptions.Compile)]
	public partial class MotorView : ContentView
	{
		public MotorView ()
		{
			InitializeComponent ();
            Up.Click    = Up_Clicked;
            Down.Click  = Down_Clicked;
		}

        //--- _test_clicked --------------------------
        private async void _test_clicked(RxButton button, bool up)
        {
            AppGlobals.Vibrate(50);     
            await Task.Delay(300);
            RxBtDef.SBtStepperTestMsg msg = new RxBtDef.SBtStepperTestMsg();
            msg.motorNo = (BindingContext as StepperMotor).No;
            msg.moveUp  = Convert.ToInt32(up);
            if (button.IsPressed) 
                AppGlobals.Bluetooth.SendMsg(RxBtDef.BT_CMD_STEPPER_TEST, ref msg);
            Debug.WriteLine("_test_clicked done");
        }
        //--- Up_Clicked ---------------------------------
        private void Up_Clicked(object sender)
        {
            _test_clicked(Up, true);
        }
        //--- Down_Clicked ---------------------------------
        private void Down_Clicked(object sender)
        {
            _test_clicked(Down, false);
        }

	}
}