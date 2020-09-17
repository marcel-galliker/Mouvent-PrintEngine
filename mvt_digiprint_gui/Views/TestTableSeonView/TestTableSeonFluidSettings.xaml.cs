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
using RX_DigiPrint.Services;

namespace RX_DigiPrint.Views.TestTableSeonView
{
	/// <summary>
	/// Interaction logic for TestTableSeonFluidSettings.xaml
	/// </summary>
	public partial class TestTableSeonFluidSettings : UserControl
	{

		public TestTableSeonFluidSettings()
		{
			InitializeComponent();

			DataContext = RxGlobals.StepperStatus;
			RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;

			C1_TestFluid.ItemsSource = new EN_TestFluid();
			Button_C1_Fluid.Content = "Waste";

			C2_TestFluid.ItemsSource = new EN_TestFluid();
			Button_C2_Fluid.Content = "Waste";

			C3_TestFluid.ItemsSource = new EN_TestFluid();
			Button_C3_Fluid.Content = "Waste";

			C4_TestFluid.ItemsSource = new EN_TestFluid();
			Button_C4_Fluid.Content = "Waste";
		}

		//--- C1_IsoXL_Clicked -------------------------------------------------
		private void C1_IsoXL_Clicked(object sender, RoutedEventArgs e)
		{
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TTS_C1_ISOP_XL);
		}

		//--- C2_IsoXL_Clicked -------------------------------------------------
		private void C2_IsoXL_Clicked(object sender, RoutedEventArgs e)
		{
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TTS_C2_ISOP_XL);
		}

		//--- C3_IsoXL_Clicked -------------------------------------------------
		private void C3_IsoXL_Clicked(object sender, RoutedEventArgs e)
		{
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TTS_C3_ISOP_XL);
		}

		//--- C4_IsoXL_Clicked -------------------------------------------------
		private void C4_IsoXL_Clicked(object sender, RoutedEventArgs e)
		{
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TTS_C4_ISOP_XL);
		}

		//--- CB_Material_DropDownClosed ----------------------------------------------
		private void TestFluid_DropDownClosed(object sender, RoutedEventArgs e)
		{

		}

		//--- Save_Clicked ---------------------------------------------
		private void Save_Clicked(object sender, RoutedEventArgs e)
		{
			if (C1_TestFluid.Text == "") C1_TestFluid.Text = "OFF";
			if (C2_TestFluid.Text == "") C2_TestFluid.Text = "OFF";
			if (C3_TestFluid.Text == "") C3_TestFluid.Text = "OFF";
			if (C4_TestFluid.Text == "") C4_TestFluid.Text = "OFF";

			int c_Index = 0;
			c_Index |= C1_TestFluid.SelectedIndex;
			c_Index |= C2_TestFluid.SelectedIndex << 2;
			c_Index |= C3_TestFluid.SelectedIndex << 4;
			c_Index |= C4_TestFluid.SelectedIndex << 6;

			_SetCtrlMode(c_Index);

		}

		//--- _SetCtrlMode ----------------
		private void _SetCtrlMode(int index)
		{
			TcpIp.SFluidTestTable msg = new TcpIp.SFluidTestTable();
			msg.index = index;
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_TTS, ref msg);
		}

		//--- PrintSystem_PropertyChanged -----------------------------------
		private void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
		{
			if (e.PropertyName.Equals("PrinterType"))
			{
				Visibility visibility;
				visibility = (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_table_seon) ? Visibility.Visible : Visibility.Collapsed;

				C1_Valves.Visibility = visibility;
				C2_Valves.Visibility = visibility;
				C3_Valves.Visibility = visibility;
				C4_Valves.Visibility = visibility;

			}

		}
	}
}
