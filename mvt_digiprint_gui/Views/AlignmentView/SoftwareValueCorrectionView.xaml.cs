using System.Windows;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;

namespace RX_DigiPrint.Views.Alignment
{
    /// <summary>
    /// Interaction logic for HeadAdjustmentView.xaml
    /// </summary>
    public partial class SoftwareValueCorrectionView : CustomWindow
    {
        private SoftwareValueCorrection _SoftwareValueCorrection;

        public SoftwareValueCorrectionView(SoftwareValueCorrection.SoftwareValueType valueType, double recommendedValue, int globalHeadNumber)
        {
            InitializeComponent();

            _SoftwareValueCorrection = new SoftwareValueCorrection(valueType, recommendedValue);

            DataContext = _SoftwareValueCorrection;
            if (globalHeadNumber>=0) TX_HeadName.DataContext = RxGlobals.HeadStat.List[globalHeadNumber];
        }

        private void SelectionLeftButton_Click(object sender, RoutedEventArgs e)
        {
            _SoftwareValueCorrection.LeftCorrectionImageSelected();
        }

        private void SelectionRightButton_Click(object sender, RoutedEventArgs e)
        {
            _SoftwareValueCorrection.RightCorrectionImageSelected();
        }
        
        private void OkayButton_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }

        private void GoBackButton_Click(object sender, RoutedEventArgs e)
        {
            _SoftwareValueCorrection.ShowSelection();
        }

        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }

        public double GetCorrectionValue()
        {
            return  _SoftwareValueCorrection.CorrectionValue.Correction;
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            RxGlobals.Screen.PlaceWindow(this);
        }
    }

}
