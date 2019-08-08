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
            CB_Overlap.ItemsSource = new EN_OnOff();
            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
        }

        //--- PrintSystem_PropertyChanged -----------------------------------------------
        void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            switch (RxGlobals.PrintSystem.PrinterType)
            {
                case EPrinterType.printer_test_table:       this.Visibility         = Visibility.Visible;
                                                            Encoder.Visibility      = Visibility.Visible;
                                                            PrintMarkDist.Visibility = Visibility.Collapsed;
                                                            Angle.Visibility        = Visibility.Visible; 
                                                            Belt.Visibility         = Visibility.Visible;
                                                            Overlap.Visibility      = Visibility.Visible;
                                                            EncoderVerso.Visibility = Visibility.Collapsed;
                                                            DistVerso.Visibility    = Visibility.Collapsed;
                                                            ManualFlightTimeComp.Visibility = Visibility.Collapsed;
                                                            break;
                case EPrinterType.printer_TX801:            
                case EPrinterType.printer_TX802:            this.Visibility         = Visibility.Visible;
                                                            Encoder.Visibility      = Visibility.Visible;
                                                            PrintMarkDist.Visibility = Visibility.Collapsed;
                                                            Angle.Visibility        = Visibility.Collapsed; 
                                                            Belt.Visibility         = Visibility.Visible; 
                                                            Overlap.Visibility      = Visibility.Visible;
                                                            EncoderVerso.Visibility = Visibility.Collapsed;
                                                            DistVerso.Visibility    = Visibility.Collapsed;
                                                            ManualFlightTimeComp.Visibility = Visibility.Visible;
                                                            break;

                case EPrinterType.printer_DP803:            this.Visibility         = Visibility.Visible;
                                                            Encoder.Visibility      = Visibility.Visible;
                                                            PrintMarkDist.Visibility = Visibility.Collapsed;
                                                            Angle.Visibility        = Visibility.Collapsed; 
                                                            Belt.Visibility         = Visibility.Collapsed;
                                                            Overlap.Visibility      = Visibility.Collapsed;
                                                            EncoderVerso.Visibility = Visibility.Visible;
                                                            DistVerso.Visibility    = Visibility.Visible;
                                                            ManualFlightTimeComp.Visibility = Visibility.Collapsed;
                                                            break;

                case EPrinterType.printer_LB701:
                case EPrinterType.printer_LB702_UV:         this.Visibility         = Visibility.Visible;
                                                            Encoder.Visibility      = Visibility.Visible;
                                                        //  PrintMarkDist.Visibility = Visibility.Collapsed;
                                                            PrintMarkDist.Visibility = Visibility.Visible;
                                                            Angle.Visibility        = Visibility.Collapsed;
                                                            Belt.Visibility         = Visibility.Collapsed;
                                                            Overlap.Visibility      = Visibility.Collapsed; 
                                                            EncoderVerso.Visibility = Visibility.Collapsed;
                                                            DistVerso.Visibility    = Visibility.Collapsed;
                                                            ManualFlightTimeComp.Visibility = Visibility.Collapsed;
                                                            break;

                case EPrinterType.printer_LH702:            this.Visibility         = Visibility.Visible;
                                                            Encoder.Visibility      = Visibility.Visible;
                                                            PrintMarkDist.Visibility = Visibility.Visible;
                                                            Angle.Visibility        = Visibility.Collapsed;
                                                            Belt.Visibility         = Visibility.Collapsed;
                                                            Overlap.Visibility      = Visibility.Collapsed; 
                                                            EncoderVerso.Visibility = Visibility.Collapsed;
                                                            DistVerso.Visibility    = Visibility.Collapsed;
                                                            ManualFlightTimeComp.Visibility = Visibility.Collapsed;
                                                            break;

                case EPrinterType.printer_LB702_WB:         this.Visibility         = Visibility.Visible;
                                                            Encoder.Visibility      = Visibility.Visible;
                                                            PrintMarkDist.Visibility = Visibility.Collapsed;
                                                            Angle.Visibility        = Visibility.Collapsed;
                                                            Belt.Visibility         = Visibility.Collapsed;
                                                            Overlap.Visibility      = Visibility.Collapsed; 
                                                            EncoderVerso.Visibility = Visibility.Collapsed;
                                                            DistVerso.Visibility    = Visibility.Visible;
                                                            ManualFlightTimeComp.Visibility = Visibility.Collapsed;
                                                            break;

                default:                                    this.Visibility         = Visibility.Visible;
                                                            Encoder.Visibility      = Visibility.Visible;
                                                            PrintMarkDist.Visibility = Visibility.Collapsed;
                                                            Angle.Visibility        = Visibility.Collapsed;
                                                            Belt.Visibility         = Visibility.Collapsed;
                                                            Overlap.Visibility      = Visibility.Collapsed; 
                                                            EncoderVerso.Visibility = Visibility.Collapsed;
                                                            DistVerso.Visibility    = Visibility.Collapsed;
                                                            ManualFlightTimeComp.Visibility = Visibility.Collapsed;
                                                            break;
            }
        }
    }
}
