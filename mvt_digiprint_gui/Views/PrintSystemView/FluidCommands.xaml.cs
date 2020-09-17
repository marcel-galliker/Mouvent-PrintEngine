using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
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

namespace RX_DigiPrint.Views.PrintSystemView
{
	/// <summary>
	/// Interaction logic for FluidCommands.xaml
	/// </summary>
	public partial class FluidCommands : UserControl
	{

		public FluidCommands()
		{
			InitializeComponent();
		}

		//--- Stop_Clicked -------------------------------------------------
		private void Flush_Clicked(object sender, RoutedEventArgs e)
		{
			Button_Flush.IsChecked = false;
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_FLUID_FLUSH);
		}

		//--- LeakTest_C1_Clicked
		private void LeakTest_C1_Clicked(object sender, RoutedEventArgs e)
		{
			Button_Leak_C1.IsChecked = false;
			TcpIp.SValue value = new TcpIp.SValue();
			value.value = 1;
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_LEAK_TEST, ref value);
		}

		//--- LeakTest_C2_Clicked
		private void LeakTest_C2_Clicked(object sender, RoutedEventArgs e)
		{
			Button_Leak_C2.IsChecked = false;
			TcpIp.SValue value = new TcpIp.SValue();
			value.value = 2;
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_LEAK_TEST, ref value);
		}

		//--- LeakTest_C3_Clicked
		private void LeakTest_C3_Clicked(object sender, RoutedEventArgs e)
		{
			Button_Leak_C3.IsChecked = false;
			TcpIp.SValue value = new TcpIp.SValue();
			value.value = 3;
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_LEAK_TEST, ref value);
		}

		//--- LeakTest_C4_Clicked
		private void LeakTest_C4_Clicked(object sender, RoutedEventArgs e)
		{
			Button_Leak_C4.IsChecked = false;
			TcpIp.SValue value = new TcpIp.SValue();
			value.value = 4;
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_LEAK_TEST, ref value);
		}
	}
}
