using RX_Common;
using RX_DigiPrint.Models;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.LB702UVView
{
    /// <summary>
    /// Interaction logic for LB701UVView.xaml
    /// </summary>
    public partial class LB702UV_View : UserControl
    {
        public LB702UV_View()
        {
            InitializeComponent();
            PlcNotConnected.DataContext = RxGlobals.Plc;
            
            RB_Main.IsChecked=true;
        }
    }
}
