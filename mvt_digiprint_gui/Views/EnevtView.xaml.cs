using Infragistics.Controls.Grids;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Windows.Controls;
using System.Windows.Media;


namespace RX_DigiPrint.Views
{
    public partial class EventView : UserControl
    {
        public EventView()
        {
            InitializeComponent();
         //   RxGlobals.Events.Dispatcher = Dispatcher;

            EventsGrid.ItemsSource = RxGlobals.Events.List;
        }

        //--- EventsGrid_CellControlAttached ---------------------------------------------------------------------------------------------------
        private void EventsGrid_CellControlAttached(object sender, Infragistics.Controls.Grids.CellControlAttachedEventArgs e)
        {
            Infragistics.Controls.Grids.CellControl ctrl=e.Cell.Control as Infragistics.Controls.Grids.CellControl;
            bool alternate = RxGlobals.Events.List[e.Cell.Row.Index].Alternate;
            if (ctrl!=null)
            { 
                switch(RxGlobals.Events.List[e.Cell.Row.Index].LogType)
                {
                case ELogType.eErrWarn:                        
                        if (alternate)  ctrl.Background    = (Brush)FindResource("XamGrid_WRN_Alternate");
                        else            ctrl.Background    = (Brush)FindResource("XamGrid_WRN_Normal");                       
                        ctrl.SetValue(Helpers.RxCellControl.MouseOverBrushProperty, (Brush)FindResource("XamGrid_WRN_MouseOver"));
                        break;

                case ELogType.eErrCont:
                case ELogType.eErrStop:
                case ELogType.eErrAbort:
                        if (alternate)  ctrl.Background    = (Brush)FindResource("XamGrid_ERR_Normal");
                        else            ctrl.Background    = (Brush)FindResource("XamGrid_ERR_Alternate");
                        ctrl.SetValue(Helpers.RxCellControl.MouseOverBrushProperty, (Brush)FindResource("XamGrid_ERR_MouseOver"));
                        break;

                default:
                        if (alternate)  ctrl.Background    = (Brush)FindResource("XamGrid_LOG_Normal");
                        else            ctrl.Background    = (Brush)FindResource("XamGrid_LOG_Alternate");
                        ctrl.SetValue(Helpers.RxCellControl.MouseOverBrushProperty, (Brush)FindResource("XamGrid_LOG_MouseOver"));
                        break;
                }
            }
        }

        //--- Confirm_clicked ----------------------------------------------------------
        private void Confirm_clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_EVT_CONFIRM);
        }

        private void ColorButton_Click(object sender, System.Windows.RoutedEventArgs e)
        {
            try
            {
                Button button = sender as Button;
                if (button != null)
                {
                    Infragistics.Controls.Grids.UnboundColumnDataContext context = button.DataContext as Infragistics.Controls.Grids.UnboundColumnDataContext;
                    LogItem item = context.RowData as LogItem;

                    if (context != null)
                    {
                        PopUpText.Text = item.Message;
                        CellControl cell = Infragistics.Windows.Utilities.GetAncestorFromType(button, typeof(CellControl), false) as CellControl;
                        if (cell != null)
                        {
                            Row row = cell.Cell.Row as Row; // <== Actual Row 
                            var cellControl = row.Cells["Message"];
                            LongTextPopUp.PlacementTarget = (System.Windows.UIElement)cellControl.Control;
                            LongTextPopUp.Width = cellControl.Control.ActualWidth;
                            LongTextPopUp.Height = 100;
                            LongTextPopUp.IsOpen = true;
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }
    }
}
