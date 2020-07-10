using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Views.Density;
using System.Windows;

namespace RX_DigiPrint.Views.AlignmentView.DisabledJets
{
    /// <summary>
    /// Interaction logic for JetCompensationDensityView.xaml
    /// </summary>
    public partial class JetCompensationDensityView : CustomWindow
    {
        private JetCompensationDensityViewModel viewModel;

        public JetCompensationDensityView(int globalHeadNumber)
        {
            InitializeComponent();
            viewModel = new JetCompensationDensityViewModel(globalHeadNumber);
            DataContext = viewModel;
        }

        private void Save_Click(object sender, RoutedEventArgs e)
        {
            DisabledJets.Save_Clicked();
            Density.Save_Clicked(sender, e);
        }

        private void Exit_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }

        private void CustomWindow_Loaded(object sender, RoutedEventArgs e)
        {
            RxGlobals.Screen.PlaceWindow(this);
        }
    }
}
