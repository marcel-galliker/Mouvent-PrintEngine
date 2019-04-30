using RX_Common;
using RX_DigiPrint.Models;
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

            DataContext = RxGlobals.Encoder;
        }

        //--- GroupBox_IsVisibleChanged ----------------------------
        private void GroupBox_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue)
            {
                if ((bool)e.NewValue) 
                {
                    RxGlobals.Encoder.Request();
                    RxGlobals.Timer.TimerFct += _Tick;
                }
                else  RxGlobals.Timer.TimerFct -= _Tick;
            }
        }

        //--- _Tick --------------------------------------------
        private void _Tick(int no)
        {
            RxGlobals.Encoder.Request();
        }

        //--- Save_Clicked ---------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxMessageBox.YesNo("Encoder Compensation", "Save new compensation parameters?", MessageBoxImage.Question, false))
                RxGlobals.Encoder.Save();
        }

    }
}
