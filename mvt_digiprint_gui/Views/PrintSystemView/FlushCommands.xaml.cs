using RX_DigiPrint.Services;
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

namespace RX_DigiPrint.Views.PrintSystemView
{
	/// <summary>
	/// Interaction logic for FlushCommands.xaml
	/// </summary>
	public partial class FlushCommands : UserControl
	{
		public FlushCommands()
		{
			InitializeComponent();
		}

		//--- Flush_C1_Clicked
		private void Flush_C1_Clicked(object sender, RoutedEventArgs e)
		{
			Button_Flush_C1.IsChecked = false;
			TcpIp.SValue value = new TcpIp.SValue();
			value.value = 1;
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_FLUSH, ref value);
		}

		//--- Flush_C2_Clicked
		private void Flush_C2_Clicked(object sender, RoutedEventArgs e)
		{
			Button_Flush_C2.IsChecked = false;
			TcpIp.SValue value = new TcpIp.SValue();
			value.value = 2;
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_FLUSH, ref value);
		}

		//--- Flush_C3_Clicked
		private void Flush_C3_Clicked(object sender, RoutedEventArgs e)
		{
			Button_Flush_C3.IsChecked = false;
			TcpIp.SValue value = new TcpIp.SValue();
			value.value = 3;
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_FLUSH, ref value);
		}

		//--- Flush_C4_Clicked
		private void Flush_C4_Clicked(object sender, RoutedEventArgs e)
		{
			Button_Flush_C4.IsChecked = false;
			TcpIp.SValue value = new TcpIp.SValue();
			value.value = 4;
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_FLUSH, ref value);
		}
	}
}
