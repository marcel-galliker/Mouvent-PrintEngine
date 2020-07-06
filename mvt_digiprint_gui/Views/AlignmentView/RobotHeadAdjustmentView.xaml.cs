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
       
        private int _inkSupplyNo;
        private int _headNo;

        public RobotHeadAdjustmentView(int inkSupplyNo, int headNo, 
            double recommendedValueAngle, double recommendedValueStitch)
        {
            InitializeComponent();

            _inkSupplyNo = inkSupplyNo;
            _headNo = headNo;

            _HeadAdjustment.Angle = recommendedValueAngle;
            _HeadAdjustment.Stitch = recommendedValueStitch;
            _HeadAdjustment.ShowHelp = false;
            
            DataContext = RxGlobals.HeadAdjustment;
        }

        private void AngleInfo_Clicked(object sender, RoutedEventArgs e)
        {
            HelpView.Init(HeadAdjustmentHelp.HeadAdjustmentType.TypeAngle,
                _HeadAdjustment.Angle, true);
            _HeadAdjustment.ShowHelp = true;
        }

        private void StitchInfo_Clicked(object sender, RoutedEventArgs e)
        {
            HelpView.Init(HeadAdjustmentHelp.HeadAdjustmentType.TypeStitch,
                _HeadAdjustment.Stitch, true);
            _HeadAdjustment.ShowHelp = true;
        }

        private void Cancel_Clicked(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }

        private void Adjust_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.HeadAdjustment.Adjust(_inkSupplyNo, _headNo);

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
