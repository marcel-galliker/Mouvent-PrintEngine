using MahApps.Metro.IconPacks;
using RX_Common;
using RX_DigiPrint.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.ConditionerTextCenterView
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
		}

		//--- started ---------------------------------------
		private void _started(MvtButton button)
		{
			actButton = button;
			actButton.IsBusy = true;
			PackIconMaterial icon = actButton.Content as PackIconMaterial;
			icon.Kind = PackIconMaterialKind.Pause;
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
