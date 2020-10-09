using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.ComponentModel;
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
    /// Interaction logic for PrintSystemGrid.xaml
    /// </summary>
    public partial class PrintSystemGrid : UserControl
    {
        public PrintSystemGrid()
        {
            InitializeComponent();

            DataContext = RxGlobals.PrintSystem;

            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
        }

		private void PrintSystem_PropertyChanged(object sender,PropertyChangedEventArgs e)
		{
			if (e.PropertyName.Equals("PrinterType"))
			{
                if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_CB612)
                    InkCylinders_RowHeight.Height = GridLength.Auto;
                else InkCylinders_RowHeight.Height = new GridLength(0);

                if (RxGlobals.PrintSystem.IsLb)
                    EncoderChoice.Visibility = EncoderType.Visibility =  Visibility.Visible;
                else
                    EncoderChoice.Visibility = EncoderType.Visibility = Visibility.Hidden;
            }
		}
	}
}
