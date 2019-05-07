using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
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

namespace RX_DigiPrint.Views.TestTableView
{
    /// <summary>
    /// Interaction logic for TestTablePrintSettings.xaml
    /// </summary>
    public partial class TestTablePrintSettings : UserControl
    {
        public TestTablePrintSettings()
        {
            InitializeComponent();

            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;

            CB_ScanMode.ItemsSource       = new EN_ScanModeList(); 
            CB_Passes.ItemsSource         = new EN_PassesList();
            CB_CuringPasses.ItemsSource   = new EN_Numbers(0, 4, 1);
            CB_Speed.ItemsSource          = new EN_SpeedTTList();
//            CB_PrEnv.ItemsSource          = RxGlobals.PrintEnv.List;
       }

        /*
        //--- Property Ripped ---------------------------------------
        private bool _Ripped;
        public bool Ripped
        {
            set 
            { if (_Ripped!=value)
                {
                    _Ripped = value;
                    if (_Ripped) MainGrid.RowDefinitions[0].Height = new GridLength(0);
                    else         MainGrid.RowDefinitions[0].Height = GridLength.Auto;
                }
            }
        }
        */
        
        //--- PrintSystem_PropertyChanged -----------------------
        private void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName=="PrinterType") CB_ScanMode.ItemsSource       = new EN_ScanModeList(); 
        }
    }
}
