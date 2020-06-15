using RX_Common;
using RX_DigiPrint.Models;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.LB701UVView
{
    /// <summary>
    /// Interaction logic for MiniLabelView.xaml
    /// </summary>
    public partial class LB701UVView : UserControl
    {
        public LB701UVView()
        {
            InitializeComponent();
            PlcNotConnected.DataContext = RxGlobals.Plc;
            
            RB_Main.IsChecked=true;
        }
    }
}
