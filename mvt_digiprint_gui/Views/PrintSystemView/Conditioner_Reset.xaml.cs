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
	/// Interaction logic for Conditioner_Reset.xaml
	/// </summary>
	public partial class Conditioner_Reset : UserControl
	{
		public Conditioner_Reset()
		{
			InitializeComponent();
		}

		//--- Reset_Cond_C1_Clicked -----------------------------------------
		private void Reset_Conditioner(object sender, RoutedEventArgs e)
		{
			Button_Cond_Reset.IsChecked = false;
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_RESET_COND);

		}
	}
}
