using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
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
        public LH702_ImgSettings()
        {
            InitializeComponent();

            CB_PrintGoMode.ItemsSource  = new EN_PgModeList();
            RxGlobals.PrintQueueChanged += _PrintQueueChanged;
        }

        private void _PrintQueueChanged()
        {
            CB_Speed.ItemsSource = RxGlobals.PrintSystem.SpeedList(RxGlobals.PrintingItem.LargestDot, RxGlobals.PrintingItem.SrcHeight);
            this.DataContext = RxGlobals.PrintingItem;
        }

        //--- Save_Clicked ---------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.PrintingItem.SendMsg(TcpIp.CMD_SET_PRINT_QUEUE);
        }

        //--- Reload_Clicked ---------------------------------------------
        private void Reload_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxGlobals.PrintingItem!=null) RxGlobals.PrintingItem.SendMsg(TcpIp.CMD_GET_PRINT_QUEUE_ITM);
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
            if (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_printing && RxGlobals.PrintingItem!=null)
            {
                RxNumBox ctrl = e.Source as RxNumBox;
                RxGlobals.PrintingItem.PageMargin = ctrl.Value;
                RxGlobals.PrintingItem.SendMsg(TcpIp.EVT_SET_PRINT_QUEUE);       
            }
        }

     }
}
