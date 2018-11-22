using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.PrintSystemView
{
    public partial class StepperGrid : UserControl
    {
        public StepperGrid()
        {
            InitializeComponent();
            DataContext = RxGlobals.Stepper;
        }
    }
}
