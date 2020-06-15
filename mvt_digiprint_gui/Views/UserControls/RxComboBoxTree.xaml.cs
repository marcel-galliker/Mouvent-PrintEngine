using Infragistics.Controls.Grids;
using Infragistics.Windows.Editors;
using RX_DigiPrint.Models;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
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
    /// Interaction logic for RxComboBoxTree.xaml
    /// </summary>
    public partial class RxComboBoxTree : UserControl
    {
        public RxComboBoxTree()
        {
            InitializeComponent();

            DataGrid.ItemsSource = RxGlobals.InkFamilies.List;
            TreePopup.CustomPopupPlacementCallback += new CustomPopupPlacementCallback(_popupPos);
        }

        //--- _popupPos ------------------------------------------
        public CustomPopupPlacement[] _popupPos(Size popupSize, Size targetSize, Point offset)
        {
            CustomPopupPlacement placement1 = new CustomPopupPlacement(new Point(0, Value.ActualHeight), PopupPrimaryAxis.Vertical);
            CustomPopupPlacement placement2 = new CustomPopupPlacement(new Point(10, 20), PopupPrimaryAxis.Horizontal);
            CustomPopupPlacement[] ttplaces = new CustomPopupPlacement[] { placement1, placement2 };
            return ttplaces;
        }

        //--- Grid_IsVisibleChanged --------------------------------------------
        private void Grid_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue)
            {
                InkSupply inkSupply = DataContext as InkSupply;
                if (inkSupply==null) return;
                foreach (Row row  in DataGrid.Rows)
                {
                    row.IsExpanded=(inkSupply.InkType!=null) && row.Data.Equals(inkSupply.InkType.Family);
                }
            }
        }

        //--- XamGrid_Loaded ---------------------------------------------------------
        private void XamGrid_Loaded(object sender, RoutedEventArgs e)
        {
            XamGrid ctrl = sender as XamGrid;
            ctrl.Columns.DataColumns["Family"].FilterColumnSettings.FilterCellValue = ctrl.DataContext;
            ctrl.Columns.DataColumns["Family"].FilterColumnSettings.FilteringOperand = new EqualsOperand();
            ctrl.ItemsSource = RxGlobals.InkTypes.List;
            InkSupply inkSupply = DataContext as InkSupply;
            if (inkSupply==null) return;
            foreach(Row row in ctrl.Rows)
            {
                row.IsSelected = row.Data.Equals(inkSupply.InkType);
            }
        }

        /*
        //--- Property IsEnabled ---------------------------------------
        private bool _IsEnabled= true;
        public bool IsEnabled
        {
            get { return _IsEnabled; }
            set { _IsEnabled=value; }
        }
        */

        //--- XamGrid_ActiveCellChanged ------------------------------------------
        private void XamGrid_ActiveCellChanged(object sender, EventArgs e)
        {
            TreePopup.IsOpen = false;
            XamGrid ctrl = sender as XamGrid;
            
            if (ctrl!=null)
            {
                InkSupply supply = DataContext as InkSupply;
                if (ctrl != null && ctrl.ActiveCell != null)
                {
                    if (ctrl.ActiveCell.Row != null && ctrl.ActiveCell.Row.Data != null)
                    {
                        supply.InkType = ctrl.ActiveCell.Row.Data as InkType;
                    }
                }
            }
        }

        //--- Unused_Clicked ----------------------------------------------
        private void Unused_Clicked(object sender, RoutedEventArgs e)
        {
            (DataContext as InkSupply).InkType=null;
            TreePopup.IsOpen = false;
        }

        //--- MyPreviewMouseDown ------------------------------------------
        private void MyPreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (IsEnabled) TreePopup.IsOpen = !TreePopup.IsOpen;
            else TreePopup.IsOpen = false;
            
            e.Handled = true;
        }

   }
}
