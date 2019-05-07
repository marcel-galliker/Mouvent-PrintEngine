using RX_Common;
using RX_DigiPrint.Models;
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
    /// Interaction logic for EncoderView.xaml
    /// </summary>
    public partial class EncoderView : UserControl
    {
        public EncoderView()
        {
            InitializeComponent();

            for (int i=0; i<RxGlobals.Encoder.Length; i++)
                if (RxGlobals.Encoder[i]==null) RxGlobals.Encoder[i] = new RX_DigiPrint.Models.Encoder(i);

            DataContext = RxGlobals.Encoder[0];
            TX_AmplOld_1.DataContext = RxGlobals.Encoder[1];
            TX_AmplNew_1.DataContext = RxGlobals.Encoder[1];
       //   TX_Percent_1.DataContext = RxGlobals.Encoder[1];
            BN_IsValid_1.DataContext = RxGlobals.Encoder[1];

            RxGlobals.PrintSystem.PropertyChanged += _PrintSystem_PropertyChanged;
        }

        //--- _PrintSystem_PropertyChanged -----------------------------------------------
        private void _PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("PrinterType"))
            {
                if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_DP803) ColEncoder2.Width = new GridLength(70);
                else ColEncoder2.Width = new GridLength(0);
            }
        }

        //--- GroupBox_IsVisibleChanged ----------------------------
        private void GroupBox_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue)
            {
                if ((bool)e.NewValue) 
                {
                    RxGlobals.Encoder[0].Request();
                    RxGlobals.Timer.TimerFct += _Tick;
                }
                else  RxGlobals.Timer.TimerFct -= _Tick;
            }
        }

        //--- _Tick --------------------------------------------
        private void _Tick(int no)
        {
            RxGlobals.Encoder[0].Request();
        }

        //--- Save_Clicked ---------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxMessageBox.YesNo("Encoder Compensation", "Save new compensation parameters?", MessageBoxImage.Question, false))
                RxGlobals.Encoder[0].Save();
        }

        //--- Save2_Clicked ---------------------------------------------
        private void Save2_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxMessageBox.YesNo("Encoder Compensation", "Save new compensation parameters?", MessageBoxImage.Question, false))
                RxGlobals.Encoder[1].Save();
        }

    }
}
