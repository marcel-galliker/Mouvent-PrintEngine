using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using System.Windows;

namespace RX_DigiPrint.Views.AlignmentView
{
    /// <summary>
    /// Interaction logic for RobotHeadAdjustmentView.xaml
    /// </summary>
    public partial class RobotHeadAdjustmentView : CustomWindow
    {
        private RX_DigiPrint.Models.HeadAdjustment _HeadAdjustment = RxGlobals.HeadAdjustment;

        public RobotHeadAdjustmentView(double recommendedValuePrintbarNo, double recommendedValueHeadNo, 
            double recommendedValueAngle, double recommendedValueStitch)
        {
            InitializeComponent();

            _HeadAdjustment.PrintbarNo = recommendedValuePrintbarNo;
            _HeadAdjustment.HeadNo = recommendedValueHeadNo;
            _HeadAdjustment.Axis = recommendedValueAngle;
            _HeadAdjustment.Steps = recommendedValueStitch;
            _HeadAdjustment.ShowHelp = false;
            
            
            ColorCnt.DataContext = RxGlobals.PrintSystem;
            HeadsPerColor.DataContext = RxGlobals.PrintSystem;
            DataContext = RxGlobals.HeadAdjustment;
        }

        private void AngleInfo_Clicked(object sender, RoutedEventArgs e)
        {
            HelpView.Init(HeadAdjustmentHelp.HeadAdjustmentType.TypeAngle,
                _HeadAdjustment.Axis, true, (int)_HeadAdjustment.PrintbarNo * RxGlobals.PrintSystem.HeadsPerColor + (int)_HeadAdjustment.HeadNo);
            _HeadAdjustment.ShowHelp = true;
        }

        private void StitchInfo_Clicked(object sender, RoutedEventArgs e)
        {
            HelpView.Init(HeadAdjustmentHelp.HeadAdjustmentType.TypeStitch,
                _HeadAdjustment.Steps, true, (int)_HeadAdjustment.PrintbarNo * RxGlobals.PrintSystem.HeadsPerColor + (int)_HeadAdjustment.HeadNo);
            _HeadAdjustment.ShowHelp = true;
        }

        private void Cancel_Clicked(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }

        private void Adjust_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.HeadAdjustment.Adjust();

            // Todo: Update ProgressBar!! (rep not implemented yet!)
            // When Adjustment is complete and successful, set correctionValues to 0!
        }

        private void Done_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.HeadAdjustment.Reset();
            DialogResult = true;
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            RxGlobals.Screen.PlaceWindow(this);
        }
    }
}
