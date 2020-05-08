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

namespace RX_DigiPrint.Views.PrintSystemView
{
    /// <summary>
    /// Interaction logic for PurgeMsg.xaml
    /// </summary>
    public partial class PurgeMsgBox : Window
    {
        public PurgeMsgBox(string text)
        {
            InitializeComponent();
            Text.Text       = text;
        }

        //--- Property Result ---------------------------------------
        private int _Result;
        public int Result
        {
            get { return _Result; }
            private set { _Result = value; }
        }
        
        //--- Yes_Clicked -----------------------------------------------
        private void Yes_Clicked(object sender, RoutedEventArgs e)
        {
            Result = 1;
            DialogResult = true;
        }

        //--- All_Clicked -------------------------------------------------
        private void All_Clicked(object sender, RoutedEventArgs e)
        {
            Result = 2;
            DialogResult = true;
        }

        //--- No_Clicked -------------------------------------------------
        private void No_Clicked(object sender, RoutedEventArgs e)
        {
            Result = 0;
            DialogResult = false;
        }

    }
}
