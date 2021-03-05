using Infragistics.Controls.Grids;
using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Helpers;
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

namespace RX_DigiPrint.Views.CleafView
{
    /// <summary>
    /// Interaction logic for CleafRolls.xaml
    /// </summary>
    public partial class CleafRolls : UserControl
    {
        public CleafRolls()
        {
            InitializeComponent();

            Rolls.ItemsSource = RxGlobals.CleafOrder.Rolls;
        }

        private void Rolls_CellClicked(object sender, CellClickedEventArgs e)
        {
            CleafRoll roll = e.Cell.Row.Data as CleafRoll;
            bool? res= MvtMessageBox.YesNoCancel(string.Format(RX_DigiPrint.Resources.Language.Resources.RollNo, roll.No), RX_DigiPrint.Resources.Language.Resources.GoodQuality, MessageBoxImage.Question, false);
            roll.send_quality(res);
        }
    }
}
