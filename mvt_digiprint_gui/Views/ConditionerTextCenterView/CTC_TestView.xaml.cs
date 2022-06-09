using Infragistics.Controls.Grids;		
using MahApps.Metro.IconPacks;
using RX_DigiPrint.Models;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.ConditionerTextCenterView
{
	/// <summary>
	/// Interaction logic for CTC_TestView.xaml
	/// </summary>
	public partial class CTC_TestView : UserControl
	{
		

		//--- constructor ------------------------------------------
		public CTC_TestView()
		{
			InitializeComponent();

			RxGlobals.CTC_TestView = this;
			TestsGrid.ItemsSource = RxGlobals.CTC_Operation.Tests;
		}

	}
}
