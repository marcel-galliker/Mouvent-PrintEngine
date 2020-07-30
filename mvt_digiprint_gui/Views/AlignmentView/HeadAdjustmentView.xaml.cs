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
            double recommendedValue, bool robotConnected, int globalHeadNumber)
        {
            HeadAdjustmentHelpView HelpView = new HeadAdjustmentHelpView();
            HelpView.Init(headAdjustmentType, recommendedValue, robotConnected, globalHeadNumber);
            MainGrid.Children.Add(HelpView);
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            RxGlobals.Screen.PlaceWindow(this);
        }

        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }
    }
}
