using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows;
using System.Windows.Input;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_Common;

namespace RX_DigiPrint.Views.Alignment
{
    /// <summary>
    /// Interaction logic for HeadAdjustmentView.xaml
    /// </summary>
    public partial class SoftwareValueCorrectionView : Window
    {
        private SoftwareValueCorrection _SoftwareValueCorrection;

        public SoftwareValueCorrectionView(SoftwareValueCorrection.SoftwareValueType valueType, double recommendedValue)
        {
            InitializeComponent();

            _SoftwareValueCorrection = new SoftwareValueCorrection(valueType, recommendedValue);

            DataContext = _SoftwareValueCorrection;
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
    }

}
