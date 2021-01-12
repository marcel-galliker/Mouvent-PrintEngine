using rx_CamLib;
using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.ComponentModel;
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
		public SetupAssistView()
		{
			InitializeComponent();

			DataContext = RxGlobals.SetupAssist;
			BTN_Continue.DataContext = RxGlobals.SA_StateMachine;
			RxGlobals.PrinterStatus.PropertyChanged += PrinterStatusChanged;
			PrinterStatusChanged(this, null);
		}

		private void PrinterStatusChanged(object sender,PropertyChangedEventArgs e)
		{
		//	StartButton.IsEnabled = (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_off || RxGlobals.PrinterStatus.PrintState==EPrintState.ps_ready_power);
			EnPlcState state = (EnPlcState)Rx.StrToInt32(RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_MACHINE_STATE"));
			StartButton.IsEnabled = state==EnPlcState.plc_stop;
		}

		//--- Settings_Clicked -------------------------------------------
		private void Settings_Clicked(object sender,RoutedEventArgs e)
		{
			SA_Settings settings = new SA_Settings(RxGlobals.Camera);
            if((bool)settings.ShowDialog()) 
			{
				CamCapture.Stop();
				CamCapture.Start(RxGlobals.Camera);
			}
		}

		//--- CamSettings_Clicked ------------------------------------
		private void CamSettings_Clicked(object sender,RoutedEventArgs e)
		{
			if (BTN_CamSettings.IsChecked) CamSettings.Hide();
			else						   CamSettings.Show(RxGlobals.Camera);
			BTN_CamSettings.IsChecked = ! BTN_CamSettings.IsChecked;
		}

		//--- Trigger_Clicked -------------------------------------------
		private void Trigger_Clicked(object sender,RoutedEventArgs e)
		{
			// RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_OUT_TRIGGER);
			SetupActions.Test();
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
			SetupActions.Start();
		}

		private void Done_Clicked(object sender,RoutedEventArgs e)
		{
			SetupActions.ActionDone();
		}

		//--- Move_Clicked -------------------------------------------
		private void Move_Clicked(object sender,RoutedEventArgs e)
		{
			RxGlobals.SetupAssist.ScanMoveTo(Rx.StrToDouble(MoveDist.Text));
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

		//--- CamCapture_IsVisibleChanged ----------------------------------
		private void CamCapture_IsVisibleChanged(object sender,DependencyPropertyChangedEventArgs e)
		{
			if((bool)e.NewValue) 
			{
				new Task(() =>
				{
					Task.Delay(100);
					RxBindable.Invoke(()=>CamCapture.Start(RxGlobals.Camera));
				}).Start();
			}
			else  CamCapture.Stop();
		}
	}
}
