using MahApps.Metro.IconPacks;
using RX_Common;
using RX_DigiPrint.Models;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.ConditionerTestCenterView
{
	/// <summary>
	/// Interaction logic for CTC_Commands.xaml
	/// </summary>
	public partial class CTC_Commands : UserControl
	{
		List<MvtButton> Button = new List<MvtButton>();
		private MvtButton actButton;

		//--- constructor ----------------------------------------------
		public CTC_Commands()
		{
			InitializeComponent();
			foreach(var item in CommandGrid.Children)
			{
				MvtButton button = item as MvtButton;
				if (button!=null)
				{
					Button.Add(button);
				}
			}
			RxGlobals.CTC_Operation.DisplayTimer = DisplayTimer;
			RxGlobals.CTC_Operation.DisplayFlow  = DisplayFlow;
		}

		//--- DisplayTimer ------------------------------------
		private void DisplayTimer(int time)
		{
			time /= 1000;
			RxBindable.Invoke(() =>
			{
				if (time > 0) TB_Time.Text = time.ToString() + "s";
				else TB_Time.Text = "";
			});
		}

		//--- DisplayFlow ---------------------------
		private void DisplayFlow(string flow)
		{
			TB_Flow.Text=flow;
		}

		//--- started ---------------------------------------
		private void _started(MvtButton button)
		{
			actButton = button;
			actButton.IsBusy = true;
			PackIconMaterial icon = actButton.Content as PackIconMaterial;
			icon.Kind = PackIconMaterialKind.Pause;
			Grid.SetRow(TB_Time, Grid.GetRow(actButton));
			for (int i = 0; i < Button.Count; i++)
			{
				if (Button[i]!=actButton) Button[i].IsEnabled=false;
			}
		}

		//--- OnDone ---------------------------------------
		private void OnDone()
		{
			if (actButton!=null) actButton.IsChecked = true;
			for(int i=0; i<Button.Count; i++)
			{
				Button[i].IsBusy	= false;
				Button[i].IsEnabled = true;
				PackIconMaterial icon = Button[i].Content as PackIconMaterial;
				icon.Kind = PackIconMaterialKind.Play;
			}
		}

		//--- ElectronicsTest_Clicked ----------------------------------------------
		private void ElectronicsTest_Clicked(object sender, RoutedEventArgs e)
		{
			_started(sender as MvtButton);
			RxGlobals.CTC_Operation.ElectronicsTest(OnDone);
		}

		//--- LeakTest_Clicked ----------------------------------------------
		private void LeakTest_Clicked(object sender, RoutedEventArgs e)
		{
			_started(sender as MvtButton);
			RxGlobals.CTC_Operation.LeakTest(OnDone);
		}

		//--- ValveTest_Clicked ----------------------------------------------
		private void ValveTest_Clicked(object sender, RoutedEventArgs e)
		{
			_started(sender as MvtButton);
			RxGlobals.CTC_Operation.ValveTest(OnDone);
		}

		//--- FlowTest_Clicked ----------------------------------------------
		private void FlowTest_Clicked(object sender, RoutedEventArgs e)
		{
			_started(sender as MvtButton);
			RxGlobals.CTC_Operation.FlowTest(OnDone);
		}

		//--- LongRun_Clicked ----------------------------------------------
		private void LongRun_Clicked(object sender, RoutedEventArgs e)
		{
			_started(sender as MvtButton);
			RxGlobals.CTC_Operation.LongRun(OnDone);
		}

		//--- Empty_Clicked ----------------------------------------------
		private void Empty_Clicked(object sender, RoutedEventArgs e)
		{
			_started(sender as MvtButton);
			RxGlobals.CTC_Operation.Empty(OnDone);
		}

		//--- Stop_Clicked ----------------------------------------------
		private void Stop_Clicked(object sender, RoutedEventArgs e)
		{
			MvtButton ctrl=sender as MvtButton;
			Point pos = ctrl.PointToScreen(new Point(0, 0));
			pos.X += ctrl.ActualWidth;
			pos.Y += ctrl.ActualHeight;
			if (MvtMessageBox.YesNoPos("STOP", "STOP test?", MessageBoxImage.Question, true, pos))
			{
				RxGlobals.CTC_Operation.Stop(OnDone);
			}
		}

		//--- Reset_Clicked ----------------------------------------------
		private void Reset_Clicked(object sender, RoutedEventArgs e)
		{
			MvtButton ctrl=sender as MvtButton;
			Point pos = ctrl.PointToScreen(new Point(0, 0));
			pos.X += ctrl.ActualWidth;
			pos.Y += ctrl.ActualHeight;
			if (MvtMessageBox.YesNoPos("Reset", "Reset tests?", MessageBoxImage.Question, true, pos))
			{
				RxGlobals.CTC_Operation.Reset();
				OnDone();
				for (int i = 0; i < Button.Count; i++)
				{
					Button[i].IsChecked = false;
				}
			}
		}
	}
}
