using rx_CamLib;
using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.SetupAssistView
{
	/// <summary>
	/// Interaction logic for SetupAssistView.xaml
	/// </summary>
	public partial class SetupAssistView :UserControl
	{
		private RxCam _Camera;

		public SetupAssistView()
		{
			InitializeComponent();

			DataContext = RxGlobals.SetupAssist;
			_Camera = new RxCam();
			_Camera.CamCallBack += new RxCam.CameraCallBack(CallBackfromCam);
		}

		//--- Settings_Clicked -------------------------------------------
		private void Settings_Clicked(object sender,RoutedEventArgs e)
		{
			SA_Settings settings = new SA_Settings(_Camera);
            if((bool)settings.ShowDialog()) 
			{
				CamCapture.Stop();
				CamCapture.Start(_Camera);
			}
		}

		//--- CamSettings_Clicked ------------------------------------
		private void CamSettings_Clicked(object sender,RoutedEventArgs e)
		{
			if (BTN_CamSettings.IsChecked) CamSettings.Hide();
			else						   CamSettings.Show(_Camera);
			BTN_CamSettings.IsChecked = ! BTN_CamSettings.IsChecked;
		}

		//--- Trigger_Clicked -------------------------------------------
		private void Trigger_Clicked(object sender,RoutedEventArgs e)
		{
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_OUT_TRIGGER);
		}

		//--- Reference_Clicked -------------------------------------------
		private void Reference_Clicked(object sender,RoutedEventArgs e)
		{
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_REFERENCE);
		}

		//--- Start_Clicked -------------------------------------------
		private void Start_Clicked(object sender,RoutedEventArgs e)
		{
			BTN_CamSettings.IsChecked = false;
			CamSettings.Hide();
			SetupActions.Start(_Camera);
		}
		private void Done_Clicked(object sender,RoutedEventArgs e)
		{
			SetupActions.ActionDone();
		}

		//--- Move_Clicked -------------------------------------------
		private void Move_Clicked(object sender,RoutedEventArgs e)
		{
			RxGlobals.SetupAssist.ScanMoveBy(Rx.StrToDouble(MoveDist.Text));
		}

		//--- Stop_Clicked -------------------------------------------
		private void Stop_Clicked(object sender,RoutedEventArgs e)
		{
			RxGlobals.SetupAssist.ScanStop();
		}

		//--- Move_Clicked -------------------------------------------
		private void WebMove_Clicked(object sender,RoutedEventArgs e)
		{
			RxGlobals.SetupAssist.WebMove();
		}
		//--- Stop_Clicked -------------------------------------------
		private void WebStop_Clicked(object sender,RoutedEventArgs e)
		{
			RxGlobals.SetupAssist.WebStop();
		}

		//--- CallBackfromCam ----------------------------------------
		private void CallBackfromCam(string CallbackData)
        {
          //  RX_Common.MvtMessageBox.Information("Camera", string.Format("Callback from Camera:\n{0}", CallbackData));
        }

		//--- CamCapture_IsVisibleChanged ----------------------------------
		private void CamCapture_IsVisibleChanged(object sender,DependencyPropertyChangedEventArgs e)
		{
			if((bool)e.NewValue) 
			{
				new Task(() =>
				{
					Task.Delay(100);
					RxBindable.Invoke(()=>CamCapture.Start(_Camera));
				}).Start();
			}
			else  CamCapture.Stop();
		}
	}
}
