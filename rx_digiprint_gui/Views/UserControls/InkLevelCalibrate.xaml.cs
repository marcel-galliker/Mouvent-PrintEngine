using RX_Common;
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
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for InkLevelCalibrate.xaml
    /// </summary>
    public partial class InkLevelCalibrate : Window
    {
        public InkLevelCalibrate(int value)
        {
            InitializeComponent();
            if (value==TcpIp.INVALID_VALUE) Value.Text = "----";
            else                            Value.Text = (value/1000.0).ToString();
        }

        //---Ok_Clicked ----------------------------------------
        private void Ok_Clicked(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }

        //---Cancel_Clicked ----------------------------------------
        private void Cancel_Clicked(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }

        //--- Property RealValue ---------------------------------------
        public int RealValue
        {
            get 
            { 
                double val=NBRealValue.Value;
                return Convert.ToInt32(val*1000); 
            }
        }        

    }
}
