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

            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
        }

        //--- PrintSystem_PropertyChanged -----------------------------------------------
        void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            switch (RxGlobals.PrintSystem.PrinterType)
            {
                case EPrinterType.printer_TX801:
                case EPrinterType.printer_TX802:    MainGrid.RowDefinitions[0].Height = new GridLength(0);  
                                                    MainGrid.RowDefinitions[1].Height = GridLength.Auto;  
                                                    MainGrid.RowDefinitions[2].Height = GridLength.Auto;  
                                                    MainGrid.RowDefinitions[3].Height = GridLength.Auto;  
                                                    break;
                
                case EPrinterType.printer_LB701:
                case EPrinterType.printer_LB702_UV: 
                case EPrinterType.printer_LB702_WB: 
                case EPrinterType.printer_DP803: 
                                                    MainGrid.RowDefinitions[0].Height = GridLength.Auto;
                                                    MainGrid.RowDefinitions[1].Height = new GridLength(0);  
                                                    MainGrid.RowDefinitions[2].Height = new GridLength(0);  
                                                    MainGrid.RowDefinitions[3].Height = new GridLength(0);  
                                                    break;

                default:                            MainGrid.RowDefinitions[0].Height = GridLength.Auto;
                                                    MainGrid.RowDefinitions[1].Height = GridLength.Auto;  
                                                    MainGrid.RowDefinitions[2].Height = GridLength.Auto;  
                                                    MainGrid.RowDefinitions[3].Height = GridLength.Auto;  
                                                    break;
            }
        }
    }
}
