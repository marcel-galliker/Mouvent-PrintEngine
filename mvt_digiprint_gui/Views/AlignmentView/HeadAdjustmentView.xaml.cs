using RX_DigiPrint.Models;
using System.ComponentModel;
using System.Windows;

namespace RX_DigiPrint.Views.Alignment
{
    /// <summary>
    /// Interaction logic for HeadAdjustment.xaml
    /// </summary>
    public partial class HeadAdjustmentView : Window
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
    }
}
