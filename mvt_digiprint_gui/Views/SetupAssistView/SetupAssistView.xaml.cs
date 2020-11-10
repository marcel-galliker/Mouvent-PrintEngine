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

		//--- UserControl_IsVisibleChanged -------------------------------------
		private void UserControl_IsVisibleChanged(object sender,DependencyPropertyChangedEventArgs e)
		{
			if((bool)e.NewValue) 
			{
				new Task(() =>
				{
					Task.Delay(500);
					RxBindable.Invoke(()=>_CamStart());
				}).Start();
			}
			else  _CamStop();
		}

		//--- Settings_Clicked -------------------------------------------
		private void Settings_Clicked(object sender,RoutedEventArgs e)
		{
			SA_Settings settings = new SA_Settings(_Camera);
            if((bool)settings.ShowDialog()) 
			{
				_CamStop();
				_CamStart();
			}
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

		//--- Move_Clicked -------------------------------------------
		private void Move_Clicked(object sender,RoutedEventArgs e)
		{
			RxGlobals.SetupAssist.Move(Rx.StrToDouble(MoveDist.Text));
		}

		//--- Stop_Clicked -------------------------------------------
		private void Stop_Clicked(object sender,RoutedEventArgs e)
		{
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_STOP);
		}

		//--- Move_Clicked -------------------------------------------
		private void WebMove_Clicked(object sender,RoutedEventArgs e)
		{
			RxGlobals.SetupAssist.WebMove();
		}
		//--- Stop_Clicked -------------------------------------------
		private void WebStop_Clicked(object sender,RoutedEventArgs e)
		{
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_WEB_STOP);
		}

		//--- CallBackfromCam ----------------------------------------
		private void CallBackfromCam(string CallbackData)
        {
          //  RX_Common.MvtMessageBox.Information("Camera", string.Format("Callback from Camera:\n{0}", CallbackData));
        }

		private void _CamStart()
		{
			_Camera.SelectCamera(RxGlobals.Settings.SetupAssistCam);
			_Camera.StartCamera(CameraCapture.Handle, false);
			_Camera.SetVideoRectangle(CameraCapture.ClientRectangle);
		}
		private void _CamStop()
		{
			_Camera.StopCamera();
		}
	}
}
