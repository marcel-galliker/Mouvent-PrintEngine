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

namespace RX_DigiPrint.Views.LH702View
{
	/// <summary>
	/// Interaction logic for LH702_Counters.xaml
	/// </summary>
	public partial class LH702_Counters :UserControl
	{
		public LH702_Counters()
		{
			InitializeComponent();

			DataContext = RxGlobals.PrinterStatus;
		}
	}
}
