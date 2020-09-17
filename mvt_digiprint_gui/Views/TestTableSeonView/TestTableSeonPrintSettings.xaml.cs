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
using RX_DigiPrint.Models.Enums;

namespace RX_DigiPrint.Views.TestTableSeonView
{
	/// <summary>
	/// Interaction logic for TestTableSeonPrintSettings.xaml
	/// </summary>
	public partial class TestTableSeonPrintSettings : UserControl
	{
		public TestTableSeonPrintSettings()
		{
			InitializeComponent();

			RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;

			CB_ScanMode.ItemsSource = new EN_ScanModeList();
			CB_Speed.ItemsSource	= new EN_SpeedTTList();
		}

		//--- PrintSystem_PropertyChanged -----------------------
		private void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
		{
			if (e.PropertyName == "PrinterType") CB_ScanMode.ItemsSource = new EN_ScanModeList();
		}
	}
}
