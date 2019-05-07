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

namespace RX_DigiPrint.Views.DP803View
{
    /// <summary>
    /// Interaction logic for MainStatus.xaml
    /// </summary>
    public partial class DP803Status : UserControl
    {
        public DP803Status()
        {
            InitializeComponent();
            DataContext = this;

            CB_MainState.ItemsSource     = new EN_MachineStateList();
        //  CB_WrinkleSensor.ItemsSource = new EN_YesNo();

            Panel.Update += TexPrepare.Update;
        }
    }
}
