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
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;


namespace RX_DigiPrint.Views.PrintSystemView
{
	/// <summary>
	/// Interaction logic for FluidSettings.xaml
	/// </summary>
	public partial class FluidSettings : UserControl
	{
		public FluidSettings()
		{
			InitializeComponent();

			DataContext = RxGlobals.StepperStatus;
		}

		private void C1_Clicked(object sender, RoutedEventArgs e)
		{
			Button_C1_OFF.IsChecked = false;
			Button_C1_IPA.IsChecked = false;
			Button_C1_XL.IsChecked = false;
			Button_C1_Waste.IsChecked = false;

			TcpIp.SValue value = new TcpIp.SValue();
			value.no = 0;
			if (sender.Equals(Button_C1_OFF))
				value.value = 0;
			else if (sender.Equals(Button_C1_IPA))
				value.value = 1;
			else if (sender.Equals(Button_C1_XL))
				value.value = 2;
			else if (sender.Equals(Button_C1_Waste))
				value.value = 3;
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_TTS, ref value);
		}


		private void C2_Clicked(object sender, RoutedEventArgs e)
		{
			Button_C2_OFF.IsChecked = false;
			Button_C2_IPA.IsChecked = false;
			Button_C2_XL.IsChecked = false;
			Button_C2_Waste.IsChecked = false;

			TcpIp.SValue value = new TcpIp.SValue();
			value.no = 1;
			if (sender.Equals(Button_C2_OFF))
				value.value = 0;
			else if (sender.Equals(Button_C2_IPA))
				value.value = 1;
			else if (sender.Equals(Button_C2_XL))
				value.value = 2;
			else if (sender.Equals(Button_C2_Waste))
				value.value = 3;
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_TTS, ref value);
		}

		private void C3_Clicked(object sender, RoutedEventArgs e)
		{
			Button_C3_OFF.IsChecked = false;
			Button_C3_IPA.IsChecked = false;
			Button_C3_XL.IsChecked = false;
			Button_C3_Waste.IsChecked = false;

			TcpIp.SValue value = new TcpIp.SValue();
			value.no = 2;
			if (sender.Equals(Button_C3_OFF))
				value.value = 0;
			else if (sender.Equals(Button_C3_IPA))
				value.value = 1;
			else if (sender.Equals(Button_C3_XL))
				value.value = 2;
			else if (sender.Equals(Button_C3_Waste))
				value.value = 3;
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_TTS, ref value);
		}

		private void C4_Clicked(object sender, RoutedEventArgs e)
		{
			Button_C4_OFF.IsChecked = false;
			Button_C4_IPA.IsChecked = false;
			Button_C4_XL.IsChecked = false;
			Button_C4_Waste.IsChecked = false;

			TcpIp.SValue value = new TcpIp.SValue();
			value.no = 3;
			if (sender.Equals(Button_C4_OFF))
				value.value = 0;
			else if (sender.Equals(Button_C4_IPA))
				value.value = 1;
			else if (sender.Equals(Button_C4_XL))
				value.value = 2;
			else if (sender.Equals(Button_C4_Waste))
				value.value = 3;
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_TTS, ref value);
		}

	}


}
