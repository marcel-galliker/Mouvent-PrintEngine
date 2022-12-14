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

            CB_Overlap.ItemsSource = new EN_OnOff();
            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
        }

        public void SetDataContext(PrintSystemExtendedView.OffsetValues offsetValues) // needed for PrintSystemExtended View
        {
            DataContext = offsetValues;
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
                Encoder.Visibility = Visibility.Collapsed;
                Angle.Visibility = Visibility.Collapsed;
                Belt.Visibility = Visibility.Collapsed;
                Overlap.Visibility = Visibility.Collapsed;
                EncoderVerso.Visibility = Visibility.Collapsed;
                DistVerso.Visibility = Visibility.Collapsed;
                ManualFlightTimeComp.Visibility = Visibility.Collapsed;
                return;
            }

            switch (type)
            {
                // TODO(CB612) add case CB612

                case EPrinterType.printer_test_table:       this.Visibility         = Visibility.Visible;
                                                            Encoder.Visibility      = Visibility.Visible;
                                                            Angle.Visibility        = Visibility.Visible; 
                                                            Belt.Visibility         = Visibility.Visible;
                                                            Overlap.Visibility      = Visibility.Visible;
                                                            EncoderVerso.Visibility = Visibility.Collapsed;
                                                            DistVerso.Visibility    = Visibility.Collapsed;
                                                            ManualFlightTimeComp.Visibility = Visibility.Collapsed;
                                                            break;
                case EPrinterType.printer_TX801:            
                case EPrinterType.printer_TX802:
                case EPrinterType.printer_TX404:            this.Visibility         = Visibility.Visible;
                                                            Encoder.Visibility      = Visibility.Visible;
                                                            Angle.Visibility        = Visibility.Collapsed; 
                                                            Belt.Visibility         = Visibility.Visible; 
                                                            Overlap.Visibility      = Visibility.Visible;
                                                            EncoderVerso.Visibility = Visibility.Collapsed;
                                                            DistVerso.Visibility    = Visibility.Collapsed;
                                                            ManualFlightTimeComp.Visibility = Visibility.Collapsed;
                                                            break;

                case EPrinterType.printer_DP803:            this.Visibility         = Visibility.Visible;
                                                            Encoder.Visibility      = Visibility.Visible;
                                                            Angle.Visibility        = Visibility.Collapsed; 
                                                            Belt.Visibility         = Visibility.Collapsed;
                                                            Overlap.Visibility      = Visibility.Collapsed;
                                                            EncoderVerso.Visibility = Visibility.Visible;
                                                            DistVerso.Visibility    = Visibility.Visible;
                                                            ManualFlightTimeComp.Visibility = Visibility.Collapsed;
                                                            break;

                case EPrinterType.printer_LB701:
                case EPrinterType.printer_LB702_UV:
                case EPrinterType.printer_Dropwatcher:      this.Visibility         = Visibility.Collapsed;
                                                            Encoder.Visibility      = Visibility.Collapsed;
                                                            Angle.Visibility        = Visibility.Collapsed;
                                                            Belt.Visibility         = Visibility.Collapsed;
                                                            Overlap.Visibility      = Visibility.Collapsed; 
                                                            EncoderVerso.Visibility = Visibility.Collapsed;
                                                            DistVerso.Visibility    = Visibility.Collapsed;
                                                            ManualFlightTimeComp.Visibility = Visibility.Collapsed;
                                                            break;

                case EPrinterType.printer_LH702:            this.Visibility         = Visibility.Collapsed;
                                                            Encoder.Visibility      = Visibility.Collapsed;
                                                            Angle.Visibility        = Visibility.Collapsed;
                                                            Belt.Visibility         = Visibility.Collapsed;
                                                            Overlap.Visibility      = Visibility.Collapsed; 
                                                            EncoderVerso.Visibility = Visibility.Collapsed;
                                                            DistVerso.Visibility    = Visibility.Collapsed;
                                                            ManualFlightTimeComp.Visibility = Visibility.Collapsed;
                                                            break;

                case EPrinterType.printer_LB702_WB:         this.Visibility         = Visibility.Visible;
                                                            Encoder.Visibility      = Visibility.Collapsed;
                                                            Angle.Visibility        = Visibility.Collapsed;
                                                            Belt.Visibility         = Visibility.Collapsed;
                                                            Overlap.Visibility      = Visibility.Collapsed; 
                                                            EncoderVerso.Visibility = Visibility.Collapsed;
                                                            DistVerso.Visibility    = Visibility.Visible;
                                                            ManualFlightTimeComp.Visibility = Visibility.Collapsed;
                                                            break;

                default:                                    this.Visibility         = Visibility.Visible;
                                                            Encoder.Visibility      = Visibility.Visible;
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
