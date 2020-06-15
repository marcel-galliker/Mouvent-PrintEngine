using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace RX_DigiPrint.Views.UserControls
{
    public partial class MouventPrinterImage : UserControl
    {
        //--- constructor ------------------------------------------
        private static bool first=true;
        public MouventPrinterImage()
        {
            InitializeComponent();
            RxGlobals.PrinterStatus.ErrorTypeChangedEvent += _ErrorTypeChanged;
            if (first)
            {
                first    = false;
                Online   = false;
                Printing = false;
           //     Error    = ELogType.eErrLog;
           //     this.Background = Brushes.Yellow;
            }
        }

        //--- Online ----------------------------------------------------
        private bool _online;
        public bool Online
        {
            get { return _online; }
            set 
            { 
                _online = value; 
                if (_online==true)
                {
                    Printer_off.Visibility=Visibility.Hidden;
                    Printer_on.Visibility=Visibility.Visible;
                    Printing=_printing;
                }
                else
                {
                    Printer_off.Visibility=Visibility.Visible;
                    Printer_on.Visibility=Visibility.Hidden;
                    Printer0.Visibility=Visibility.Hidden;
                    Printer1.Visibility=Visibility.Hidden;
                    Printer2.Visibility=Visibility.Hidden;
                    Printer3.Visibility=Visibility.Hidden;
                    Printer4.Visibility=Visibility.Hidden;
                }
            }
        }
        
        //--- printing -------------------------------------------------
        private bool _printing;
        public bool Printing
        {
            get { return _printing; }
            set 
            { 
                _printing = value;
                if (_online==true)
                { 
                    if (_printing==true)
                    {
                        Printer_off.Visibility=Visibility.Hidden;
                        Printer_on.Visibility=Visibility.Hidden;
                        Printer0.Visibility=Visibility.Visible;
                        Printer1.Visibility=Visibility.Visible;
                        Printer2.Visibility=Visibility.Visible;
                        Printer3.Visibility=Visibility.Visible;
                        Printer4.Visibility=Visibility.Visible;
                    }
                    else
                    {
                        Printer_off.Visibility=Visibility.Hidden;
                        Printer_on.Visibility=Visibility.Visible;
                        Printer0.Visibility=Visibility.Hidden;
                        Printer1.Visibility=Visibility.Hidden;
                        Printer2.Visibility=Visibility.Hidden;
                        Printer3.Visibility=Visibility.Hidden;
                        Printer4.Visibility=Visibility.Hidden;
                    }
                 }
            }
        }

        //--- ErrorTypeChanged -------------------------------------
        private void _ErrorTypeChanged(ELogType errorType)
        {
            switch(errorType)
            {
                case ELogType.eErrLog:   this.Background= Brushes.Transparent; break;
                case ELogType.eErrWarn:  this.Background= Brushes.Gold; break;
                case ELogType.eErrCont:  this.Background= Brushes.Crimson; break;
                case ELogType.eErrStop: this.Background = Brushes.Crimson; break;
                case ELogType.eErrAbort: this.Background = Brushes.Crimson; break;
            }
        }
        
    }
}
