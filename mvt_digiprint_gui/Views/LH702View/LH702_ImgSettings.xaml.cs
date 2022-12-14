using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace RX_DigiPrint.Views.LH702View
{
    /// <summary>
    /// Interaction logic for MainPar.xaml
    /// </summary>
    public partial class LH702_ImgSettings : UserControl
    {
        private PrintQueueItem _Item;

        public LH702_ImgSettings()
        {
            InitializeComponent();

            CB_PrintGoMode.ItemsSource  = new EN_PgModeList();
            RowCopies.Height = new GridLength(0);
        }

        //--- Property CanDelete ---------------------------------------
        public bool CanDelete
        {
            set 
            { 
                if (value) ButtonDelete.Visibility = Visibility.Visible;
                else       ButtonDelete.Visibility = Visibility.Collapsed;
            }
        }
        
        //--- _DataContextChanged -
        private void _DataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            _Item = DataContext as PrintQueueItem;
        }

        //--- Save_Clicked ---------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            if (_Item!=null)
            {
                _Item.ScanLength = _Item.Copies;
                _Item.SendMsg(TcpIp.CMD_SET_PRINT_QUEUE);
                _Item.SaveDefaults();
            }
        }

        //--- Reload_Clicked ---------------------------------------------
        private void Reload_Clicked(object sender, RoutedEventArgs e)
        {
            if (_Item!=null) _Item.SendMsg(TcpIp.CMD_GET_PRINT_QUEUE_ITM);
        }

        //--- Delete_Clicked ---------------------------------------------
        private void Delete_Clicked(object sender, RoutedEventArgs e)
        {
            if (_Item!=null)
            {
                if (MvtMessageBox.YesNo("Delete", "Delete the Items",  MessageBoxImage.Question, false))
                {
                    _Item.SendMsg(TcpIp.CMD_DEL_PRINT_QUEUE);
                }
            }
        }

        //--- Grid_Loaded -------------------------
        private void Grid_Loaded(object sender, RoutedEventArgs e)
        {
            Grid grid = sender as Grid;
            int i;
            for (i=0; i<ParamsGrid.Children.Count; i++)
            {
                if (ParamsGrid.Children[i].Equals(sender))
                {
                    if ((i&1)==0) grid.Background=Application.Current.Resources["XamGrid_Alternate"] as Brush;
                    else          grid.Background=Brushes.Transparent;
                }
            }
        }

        //--- PageMargin_LostFocus -----------------------------------------------------------
        private void PageMargin_LostFocus(object sender, RoutedEventArgs e)
        {
            if (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_printing && _Item!=null)
            {
                MvtNumBox ctrl = e.Source as MvtNumBox;
                _Item.PageMargin = ctrl.Value;
                _Item.SendMsg(TcpIp.EVT_SET_PRINT_QUEUE);       
            }
        }

        //--- Copies_LostFocus -----------------------------------------------------------
        private void Copies_LostFocus(object sender, RoutedEventArgs e)
        {
            if (RxGlobals.PrinterStatus.PrintState == EPrintState.ps_printing && _Item != null)
            {
                RxNumBox ctrl = e.Source as RxNumBox;
                _Item.Copies = (Int32)(ctrl.Value + 0.5);
                _Item.SendMsg(TcpIp.EVT_SET_PRINT_QUEUE);
            }
        }

        //--- CB_PrintGoMode_SelectionChanged -----------------------------------
        private void CB_PrintGoMode_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            TcpIp.EPrintGoMode mode;
            if (_Item==null) return;
            if (CB_PrintGoMode.SelectedValue==null)
                mode = _Item.PrintGoMode;
            else
                mode = (TcpIp.EPrintGoMode)CB_PrintGoMode.SelectedValue;
            if (mode==TcpIp.EPrintGoMode.PG_MODE_GAP || mode==TcpIp.EPrintGoMode.PG_MODE_LENGTH)
                RowCopies.Height = GridLength.Auto;
            else
                RowCopies.Height = new GridLength(0);
        }

     }
}
