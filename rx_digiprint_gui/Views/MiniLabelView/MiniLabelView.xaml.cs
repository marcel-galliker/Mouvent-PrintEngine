using RX_Common;
using RX_DigiPrint.Models;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.MiniLabelView
{
    /// <summary>
    /// Interaction logic for MiniLabelView.xaml
    /// </summary>
    public partial class MiniLabelView : UserControl
    {
        public MiniLabelView()
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
