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
        }

        public void SetDataContext()
        {
            DataContext = RxGlobals.Stepper;
            ManualFlightTimeComp.DataContext = RxGlobals.PrintSystem;

            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
        }

        public void SetDataContext(PrintSystemExtendedView.StepperValues stepperValues)
        {
            DataContext = stepperValues;
        }

        //--- PrintSystem_PropertyChanged -----------------------------------------------
        void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            UpdateVisibility(RxGlobals.PrintSystem.PrinterType);
        }

        public void UpdateVisibility(EPrinterType type)
        {
            if (!RxGlobals.PrintSystem.ExpandSettingsPanel)
            {
                this.Visibility = Visibility.Collapsed;
                return;
            }

            this.Visibility = Visibility.Visible;

            switch (type)
            {
                // TODO(CB612) add case for CB612

                // row 0: RefHeight
                // row 1: WipeHeight
                // row 2: CapHeight
                // row 3: AdjustPos
                // row 4: FlightTime

                // TX80x -> no Stepper Settings grid, code unused?
                case EPrinterType.printer_TX801:
                case EPrinterType.printer_TX802:
                case EPrinterType.printer_TX404:    MainGrid.RowDefinitions[0].Height = new GridLength(0);
                                                    MainGrid.RowDefinitions[1].Height = GridLength.Auto;  
                                                    MainGrid.RowDefinitions[2].Height = GridLength.Auto;  
                                                    MainGrid.RowDefinitions[3].Height = GridLength.Auto;  
                                                    break;

                // printer_LB702_UV, printer_LB702_WB, printer_LH702, printer_DP803  -> other Stepper Settings grids, code only used for LB701?
                case EPrinterType.printer_LB701:
                case EPrinterType.printer_LB702_UV: 
                case EPrinterType.printer_LB702_WB: 
                case EPrinterType.printer_LH702: 
                case EPrinterType.printer_DP803: 
                                                    MainGrid.RowDefinitions[0].Height = GridLength.Auto;
                                                    MainGrid.RowDefinitions[1].Height = new GridLength(0);  
                                                    MainGrid.RowDefinitions[2].Height = new GridLength(0);  
                                                    MainGrid.RowDefinitions[3].Height = new GridLength(0);  
                                                    break;

                // printer_undef, printer_test_table, printer_test_slide, printer_test_slide_only
                default:                            MainGrid.RowDefinitions[0].Height = GridLength.Auto;
                                                    MainGrid.RowDefinitions[1].Height = GridLength.Auto;  
                                                    MainGrid.RowDefinitions[2].Height = GridLength.Auto;  
                                                    MainGrid.RowDefinitions[3].Height = GridLength.Auto;  
                                                    break;
            }
        }
    }
}
