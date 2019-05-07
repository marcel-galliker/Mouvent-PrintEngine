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
    /// Interaction logic for PrintSystemGrid.xaml
    /// </summary>
    public partial class PrintSystemGrid : UserControl
    {
        public PrintSystemGrid()
        {
            InitializeComponent();

            DataContext = RxGlobals.PrintSystem;
        }
    }
}
