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
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Services;

namespace RX_DigiPrint.Views.UserControls
{
	/// <summary>
	/// Interaction logic for SetClusterNo.xaml
	/// </summary>
	public partial class SetClusterNo : CustomWindow
	{
		public SetClusterNo()
		{
			InitializeComponent();

			DataContext = RxGlobals.ClusterNo;
		}

		private void C1_SaveNo(object sender, RoutedEventArgs e)
		{
			RxGlobals.ClusterNo.Set_ClusterNo(1);
		}

		private void C2_SaveNo(object sender, RoutedEventArgs e)
		{
			RxGlobals.ClusterNo.Set_ClusterNo(2);
		}

		private void C3_SaveNo(object sender, RoutedEventArgs e)
		{
			RxGlobals.ClusterNo.Set_ClusterNo(3);
		}

		private void C4_SaveNo(object sender, RoutedEventArgs e)
		{
			RxGlobals.ClusterNo.Set_ClusterNo(4);
		}

		private void Cancel_Clicked(object sender, RoutedEventArgs e)
		{
			DialogResult = false;
		}
	}
}
