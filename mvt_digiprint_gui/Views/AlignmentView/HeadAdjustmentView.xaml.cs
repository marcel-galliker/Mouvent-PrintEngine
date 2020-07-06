using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using System.Windows;

namespace RX_DigiPrint.Views.Alignment
{
    /// <summary>
    /// Interaction logic for HeadAdjustment.xaml
    /// </summary>
    public partial class HeadAdjustmentView : CustomWindow
    {
        public HeadAdjustmentView()
        {
            InitializeComponent();
        }

        public void Init(HeadAdjustmentHelp.HeadAdjustmentType headAdjustmentType,
            double recommendedValue, bool robotConnected)
        {
            HeadAdjustmentHelpView HelpView = new HeadAdjustmentHelpView();
            HelpView.Init(headAdjustmentType, recommendedValue, robotConnected);
            MainGrid.Children.Add(HelpView);
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            RxGlobals.Screen.PlaceWindow(this);
        }
    }
}
