using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System.Globalization;
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
    public partial class LH702_Par : UserControl
    {
        public LH702_Par()
        {
            InitializeComponent();

            this.DataContext = RxGlobals.MaterialXML;

            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
        }

        //--- PrintSystem_PropertyChanged ----------------------------------------------------
        void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
        //    if (e.PropertyName.Equals("SpeedList"))
        //         CB_Speed.ItemsSource  = RxGlobals.PrintSystem.SpeedList;
        }

        //--- Save_Clicked ---------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
        //    CB_Material.EndEditMode(true, true);
            RxGlobals.MaterialList.List[0].SaveValue("PAR_HEAD_HEIGHT", RxGlobals.MaterialXML.PrintHeight.ToString(new CultureInfo("en-US")));
            RxGlobals.MaterialList.List[0].Send(TcpIp.CMD_PLC_SAVE_MATERIAL);
        }

        //--- Reload_Clicked ---------------------------------------------
        private void Reload_Clicked(object sender, RoutedEventArgs e)
        {
//            ParPanelMaterial.Reset();
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

     }
}
