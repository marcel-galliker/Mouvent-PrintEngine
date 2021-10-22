using RX_Common;
using RX_DigiPrint.Devices;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
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

		//--- PrinterStatusChanged ---------------------------------------------
		private void PrinterStatusChanged(object sender,PropertyChangedEventArgs e)
		{
			EnPlcState state = (EnPlcState)Rx.StrToInt32(RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_MACHINE_STATE"));
		//	StartAlignButton.IsEnabled = (state==EnPlcState.plc_stop || state==EnPlcState.plc_error)|| RxGlobals.SA_StateMachine.SimuMachine;
			StartAlignButton.IsEnabled = (state==EnPlcState.plc_stop)|| RxGlobals.SA_StateMachine.SimuMachine;
			StartDensityButton.IsEnabled = (state==EnPlcState.plc_stop)|| RxGlobals.SA_StateMachine.SimuMachine;
			StartRegisterButton.IsEnabled = (state==EnPlcState.plc_stop)|| RxGlobals.SA_StateMachine.SimuMachine;
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
			if (BTN_CamSettings.IsChecked) BTN_CamSettings.IsChecked=CamSettings.Hide();
			else BTN_CamSettings.IsChecked=CamSettings.Show(RxGlobals.Camera);
		}

		//--- Reference_Clicked -------------------------------------------
		private void Reference_Clicked(object sender,RoutedEventArgs e)
		{
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_REFERENCE);
		}

		//--- Up_Clicked -------------------------------------------
		private void Up_Clicked(object sender,RoutedEventArgs e)
		{
			RxGlobals.SetupAssist.Up();
		}

		//--- Down_Clicked -------------------------------------------
		private void Down_Clicked(object sender,RoutedEventArgs e)
		{
			RxGlobals.SetupAssist.Down();
		}

		private void SlideLeft_Clicked(object sender,RoutedEventArgs e)
		{
			RxGlobals.SetupAssist.ScanMoveTo(100, 50);
		}
		private void SlideRight_Clicked(object sender,RoutedEventArgs e)
		{
			RxGlobals.SetupAssist.ScanMoveTo(20, 50);
		}

		//--- StartAlign_Clicked -------------------------------------------
		private void StartAlign_Clicked(object sender,RoutedEventArgs e)
		{
			BTN_CamSettings.IsChecked = false;
			CamSettings.Hide();
			SetupActions.StartAlign();
		}

		//--- StartDensity_Clicked -------------------------------------------
		private void StartDensity_Clicked(object sender,RoutedEventArgs e)
		{
			BTN_CamSettings.IsChecked = false;
			CamSettings.Hide();
			SetupActions.StartDensity();
		}

		//--- StartRegister_Clicked -------------------------------------------
		private void StartRegister_Clicked(object sender,RoutedEventArgs e)
		{
			BTN_CamSettings.IsChecked = false;
			CamSettings.Hide();
			SetupActions.StartRegister();
		}

		//--- Report_Clicked ----------------------------------------------
		private void Report_Clicked(object sender,RoutedEventArgs e)
		{
			SetupActions.PrintReport();
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
