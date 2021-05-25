using rx_CamLib;
using RX_Common;
using RX_DigiPrint.Devices;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
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

			DataContext				 = RxGlobals.SetupAssist;
			BTN_Continue.DataContext = RxGlobals.SA_StateMachine;
			RxGlobals.PrinterStatus.PropertyChanged += PrinterStatusChanged;
			PrinterStatusChanged(this, null);
		}

		//--- _StepTextBox ------------------------
		private TextBox _StepTextBox(int stepper, int row, string property)
		{
			TextBox box = new TextBox();
			Grid.SetColumn(box, stepper+1);
			Grid.SetRow   (box, row);
			box.IsReadOnly  = true;
			if (stepper<0)
			{
				box.Text = property;
			}
			else
			{
				box.DataContext = RxGlobals.StepperStatus[stepper];
				box.SetBinding(TextBox.TextProperty, property);
			}
			return box;
		}

		private void PrinterStatusChanged(object sender,PropertyChangedEventArgs e)
		{
			EnPlcState state = (EnPlcState)Rx.StrToInt32(RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_MACHINE_STATE"));
		//	StartButton.IsEnabled = (state==EnPlcState.plc_stop || state==EnPlcState.plc_error)|| RxGlobals.SA_StateMachine.SimuMachine;
			StartButton.IsEnabled = (state==EnPlcState.plc_stop)|| RxGlobals.SA_StateMachine.SimuMachine;
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
			CamSettings.Hide();
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

		//--- Stop_Clicked -------------------------------------------
		private void WebStop_Clicked(object sender,RoutedEventArgs e)
		{
			RxGlobals.SetupAssist.WebStop();
		}

		//--- CamCapture_IsVisibleChanged ----------------------------------
		private bool _CamRunning=false;
		private void CamCapture_IsVisibleChanged(object sender,DependencyPropertyChangedEventArgs e)
		{
			if((bool)e.NewValue)
			{
				if (!_CamRunning)
				{
					_CamRunning=true;
					new Task(() =>
					{
						Task.Delay(100);
						RxBindable.Invoke(()=>CamCapture.Start(RxGlobals.Camera));
					}).Start();
				}
			}
			else  
			{
				if (!RxGlobals.SA_StateMachine.IsRunning)
				{
					CamCapture.Stop();
					_CamRunning=false;
				}
			}
		}

		//--- I1Pro3_Connect ---------------------------------------------------
		private void I1Pro3_Connect(object sender,RoutedEventArgs e)
		{
			this.Cursor = System.Windows.Input.Cursors.Wait;
			if (RxGlobals.I1Pro3.Connect())
			{
				BTN_I1Pro3_Calibrate.IsEnabled = true;
			}
			else 
			{
				BTN_I1Pro3_Calibrate.IsEnabled = false;
				MessageBox.Show("No I1Pro3 Spectrometer found", "Connect to Spectrometer");
			}
			this.Cursor = System.Windows.Input.Cursors.Arrow;
		}

		//--- I1Pro3_Calibrate ---------------------------------------------------
		private void I1Pro3_Calibrate(object sender,RoutedEventArgs e)
		{
			this.Cursor = System.Windows.Input.Cursors.Wait;
            BTN_I1Pro3_Measure.IsEnabled = RxGlobals.I1Pro3.WhiteCalibrate();
			this.Cursor = System.Windows.Input.Cursors.Arrow;
		}

		//--- I1Pro3_Measure ---------------------------------------------------
		private ColorConversion.SpectroResultStruct SpectroResult;
		private void I1Pro3_Measure(object sender,RoutedEventArgs e)
		{
			ColorConversion.SpectroResultStruct last=SpectroResult;
            RxGlobals.I1Pro3.MeasurePoint(ref SpectroResult);

			ResultCMYK.Text = "C "+SpectroResult.Density.C.ToString("F2")+"\n"
							+ "M "+SpectroResult.Density.M.ToString("F2")+"\n"
							+ "Y "+SpectroResult.Density.Y.ToString("F2")+"\n"
							+ "K "+SpectroResult.Density.K.ToString("F2");

			ResultLAB.Text  = "L "+SpectroResult.CieLab.L.ToString("F2")+"\n"
							+ "a "+SpectroResult.CieLab.a.ToString("F2")+"\n"
							+ "b "+SpectroResult.CieLab.b.ToString("F2");
		
			System.Windows.Media.Color color = ColorConversion.CieLabToRgb(SpectroResult.CieLab);
			ResultColor.Fill = new System.Windows.Media.SolidColorBrush(color);

			ResultDE.Text = "ΔE₂₀₀₀ " + ColorConversion.DeltaE2000(SpectroResult.CieLab, last.CieLab).ToString("F3");
		}
	}
}
