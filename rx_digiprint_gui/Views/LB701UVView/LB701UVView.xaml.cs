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
            DiePos.Visibility = Visibility.Collapsed;
        }

        //--- PositionDie_Clicked -------------------------
        private void PositionDie_Clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            RxButton but = sender as RxButton;
            if (but!=null)
            {
                but.IsChecked = !but.IsChecked;
                if (but.IsChecked)
                {
                    DiePos.Visibility = Visibility.Visible;
                }
                else
                {
                    DiePos.Visibility = Visibility.Collapsed;
                }
            }
        }
    }
}
