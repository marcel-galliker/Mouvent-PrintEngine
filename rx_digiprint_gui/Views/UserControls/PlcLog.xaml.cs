using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using RxRexrothGui.Models;
using RxRexrothGui.Services;
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

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for PlcLog.xaml
    /// </summary>
    public partial class PlcLog : UserControl
    {
        public PlcLog()
        {
            InitializeComponent();
            DataContext = this;
            LogGrid.ItemsSource = RxGlobals.Plc.Log;
        }

        //--- LogGrid_CellControlAttached -------------------------------------------
        private void LogGrid_CellControlAttached(object sender, Infragistics.Controls.Grids.CellControlAttachedEventArgs e)
        {
            Infragistics.Controls.Grids.CellControl ctrl=e.Cell.Control as Infragistics.Controls.Grids.CellControl;
          
            if (ctrl!=null)
            { 
                if ((e.Cell.Row.Index&0x01)!=0)  ctrl.Background = (Brush)FindResource("XamGrid_LOG_Normal");
                else                             ctrl.Background = (Brush)FindResource("XamGrid_LOG_Alternate");

                ctrl.SetValue(Helpers.RxCellControl.MouseOverBrushProperty, (Brush)FindResource("XamGrid_LOG_MouseOver"));
            }
        }

    }
}
