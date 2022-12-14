using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.PrintSystemView
{
    /// <summary>
    /// Interaction logic for EncoderView.xaml
    /// </summary>

    public partial class EncoderView : UserControl
    {
        public bool HasEncoder2 { get; set; }
        public EncoderView()
        {
            InitializeComponent();

            
        }

        public void SetDataContext()
        {
            for (int i = 0; i < RxGlobals.Encoder.Length; i++)
                if (RxGlobals.Encoder[i] == null) RxGlobals.Encoder[i] = new RX_DigiPrint.Models.Encoder(i);

            DataContext = RxGlobals.Encoder[0];
            TX_AmplOld_1.DataContext = RxGlobals.Encoder[1];
            TX_AmplNew_1.DataContext = RxGlobals.Encoder[1];
            BN_IsValid_1.DataContext = RxGlobals.Encoder[1];
            HasEncoder2 = RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_DP803 ? true : false;
            RxGlobals.PrintSystem.PropertyChanged += _PrintSystem_PropertyChanged;
        }

        public void SetDataContext(PrintSystemExtendedView.EncoderValues encoder0, PrintSystemExtendedView.EncoderValues encoder1)
        {
            DataContext = encoder0;
            if (encoder1 != null)
            {
                TX_AmplOld_1.DataContext = encoder1;
                TX_AmplNew_1.DataContext = encoder1;
                BN_IsValid_1.DataContext = encoder1;
            }
        }

        //--- _PrintSystem_PropertyChanged -----------------------------------------------
        private void _PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("PrinterType"))
            {
                UpdateVisibility(RxGlobals.PrintSystem.PrinterType);
            }
        }

        public void UpdateVisibility(EPrinterType type)
        {
            // TODO(CB612) add case for CB612

            if (!RxGlobals.PrintSystem.ExpandSettingsPanel)
            {
                this.Visibility = Visibility.Collapsed;
                return;
            }
            this.Visibility = Visibility.Visible;

            if (type == EPrinterType.printer_DP803)
            {
                ColEncoder2.Width = new GridLength(70);
                HasEncoder2 = true;
            }
            else
            {
                ColEncoder2.Width = new GridLength(0);
                HasEncoder2 = false;
            }
        }

        //--- Save_Clicked ---------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            if (MvtMessageBox.YesNo("Encoder Compensation", "Save new compensation parameters?", MessageBoxImage.Question, false))
                RxGlobals.Encoder[0].Save();
        }

        //--- Save2_Clicked ---------------------------------------------
        private void Save2_Clicked(object sender, RoutedEventArgs e)
        {
            if (MvtMessageBox.YesNo("Encoder Compensation", "Save new compensation parameters?", MessageBoxImage.Question, false))
                RxGlobals.Encoder[1].Save();
        }

    }
}
