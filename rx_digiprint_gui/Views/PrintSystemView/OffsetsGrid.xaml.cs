using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.PrintSystemView
{
    /// <summary>
    /// Interaction logic for OffsetsGrid.xaml
    /// </summary>
    public partial class OffsetsGrid : UserControl
    {
        public OffsetsGrid()
        {
            InitializeComponent();

            DataContext = RxGlobals.PrintSystem;
            CB_Overlap.ItemsSource = new EN_YesNo();
            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
        }

        //--- PrintSystem_PropertyChanged -----------------------------------------------
        void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            Encoder.Visibility = Visibility.Visible;
            switch (RxGlobals.PrintSystem.PrinterType)
            {
                case EPrinterType.printer_test_table:       Angle.Visibility    = Visibility.Visible; 
                                                            Belt.Visibility     = Visibility.Visible;
                                                            Overlap.Visibility  = Visibility.Collapsed;
                                                            Verso.Visibility    = Visibility.Collapsed;
                                                            break;
                case EPrinterType.printer_TX801:            
                case EPrinterType.printer_TX802:            Angle.Visibility    = Visibility.Collapsed; 
                                                            Belt.Visibility     = Visibility.Visible; 
                                                            Overlap.Visibility  = Visibility.Visible;
                                                            Verso.Visibility    = Visibility.Collapsed;
                                                            break;

                case EPrinterType.printer_DP803:            Angle.Visibility    = Visibility.Collapsed; 
                                                            Belt.Visibility     = Visibility.Collapsed;
                                                            Overlap.Visibility  = Visibility.Collapsed;
                                                            Verso.Visibility    = Visibility.Visible;
                                                            break;

                default:                                    Angle.Visibility    = Visibility.Collapsed;
                                                            Belt.Visibility     = Visibility.Collapsed;
                                                            Overlap.Visibility  = Visibility.Collapsed; 
                                                            Verso.Visibility    = Visibility.Collapsed;
                                                            break;
            }
        }

    }
}
