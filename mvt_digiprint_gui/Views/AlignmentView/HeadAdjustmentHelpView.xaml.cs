using System;
using System.Windows;
using RX_DigiPrint.Models;
using RX_Common;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.Alignment
{
    /// <summary>
    /// Interaction logic for HeadAdjustmentView.xaml
    /// </summary>
    public partial class HeadAdjustmentHelpView : UserControl
    {
        private HeadAdjustmentHelp _HeadAdjustmentHelp = new HeadAdjustmentHelp();
        private HeadAdjustment _HeadAdjustment = RxGlobals.HeadAdjustment;

        public HeadAdjustmentHelpView()
        {
            InitializeComponent();
        }
        
        public void Init(HeadAdjustmentHelp.HeadAdjustmentType headAdjustmentType,
            double recommendedValue, bool robotConnected, int globalHeadNumber)
        {
            _HeadAdjustmentHelp.Init(headAdjustmentType, recommendedValue, robotConnected);
            _CreateInstructionsImagePanel();

            DataContext = _HeadAdjustmentHelp;
            TX_HeadName.DataContext = RxGlobals.HeadStat.List[globalHeadNumber];
        }

        private void SelectionLeftButton_Click(object sender, RoutedEventArgs e)
        {
            _HeadAdjustmentHelp.LeftCorrectionImageSelected();
            _CreateInstructionsImagePanel();
        }

        private void SelectionRightButton_Click(object sender, RoutedEventArgs e)
        {
            _HeadAdjustmentHelp.RightCorrectionImageSelected();
            _CreateInstructionsImagePanel();
        }
        
        private void _CreateInstructionsImagePanel()
        {
            if (_HeadAdjustmentHelp.FaultType == RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeUndefined)
            {
                return;
            }

            InstructionsImageStackPanel.Children.Clear();

            try
            {
                if (_HeadAdjustmentHelp.FaultType == RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue)
                {
                    var imageSource = _HeadAdjustmentHelp.InstructionsImageSourceList[AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                    for (int i = 0; i < imageSource.Count; i++)
                    {
                        InstructionsView view = new InstructionsView(imageSource[i].ImageSource, imageSource[i].ImageHeader);
                        InstructionsImageStackPanel.Children.Add(view);
                    }
                }
                else // AlignmentFaultType.TypeRight
                {
                    var imageSource = _HeadAdjustmentHelp.InstructionsImageSourceList[AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                    for (int i = 0; i < imageSource.Count; i++)
                    {
                        InstructionsView view = new InstructionsView(imageSource[i].ImageSource, imageSource[i].ImageHeader);
                        InstructionsImageStackPanel.Children.Add(view);
                    }
                }
            }
            catch (Exception)
            {
                // software internal problem!
                MvtMessageBox.InformationAndExit("Error", "");
            }
        }

        private void OkayButton_Click(object sender, RoutedEventArgs e)
        {
            _HeadAdjustmentHelp.OnOkay();
            if(_HeadAdjustmentHelp.AdjustmentType == HeadAdjustmentHelp.HeadAdjustmentType.TypeAngle)
                _HeadAdjustment.Angle = _HeadAdjustmentHelp.CorrectionValue;
            else if (_HeadAdjustmentHelp.AdjustmentType == HeadAdjustmentHelp.HeadAdjustmentType.TypeStitch)
                _HeadAdjustment.Stitch = _HeadAdjustmentHelp.CorrectionValue;
            _HeadAdjustment.ShowHelp = false;
        }

        private void GoBackButton_Click(object sender, RoutedEventArgs e)
        {
            _HeadAdjustmentHelp.ShowSelection();
        }

        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            _HeadAdjustment.ShowHelp = false;
        }
    }

}
