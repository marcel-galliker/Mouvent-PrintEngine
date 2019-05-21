using RX_DigiPrint.Models;
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
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.PrintSystemView
{
    /// <summary>
    /// Interaction logic for InkSupplyGrid.xaml
    /// </summary>
    public partial class InkSupplyGrid : UserControl
    {
        private InkSupplyList               _InkSupply     = RxGlobals.InkSupply;
        private List<InkSupplyView>         _InkSupplyView = new List<InkSupplyView>();

        //--- Constructor -------------------------------
        public InkSupplyGrid()
        {
            InitializeComponent();

            for (int i=0; i<TcpIp.InkSupplyCnt; i++)
            { 
                _InkSupplyView.Add(new InkSupplyView());
                _InkSupplyView[i].DataContext = _InkSupply.List[i];
                InkSupplyStack.Children.Add(_InkSupplyView[i]);
            }
                
            TX_LungPressure.DataContext = RxGlobals.InkSupply.List[0];

            show_inkSupplies(RxGlobals.PrintSystem.ColorCnt);
            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
            RxGlobals.User.PropertyChanged += User_PropertyChanged;
            User_PropertyChanged(null, null);
        }

        //--- User_PropertyChanged --------------------------------------
        private void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            Visibility visibility  =  (RxGlobals.User.UserType >= EUserType.usr_service) ? Visibility.Visible : Visibility.Collapsed; 
            ServiceGrid.Visibility = visibility;
        }

        //--- PrintSystem_PropertyChanged ----------------------------------
        private void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("PrinterType")) _printertype_changed();
            if (e.PropertyName.Equals("Reverse")) _assign_inksupply(RxGlobals.PrintSystem.ColorCnt);
        }

        //--- _printertype_changed -----------------------------------------
        private void _printertype_changed()
        {
            int i;
            TX_Side.Visibility = RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_DP803? Visibility.Visible : Visibility.Collapsed;
            InkSuplyGrid.RowDefinitions[2].Height = new GridLength(25/RxGlobals.Screen.Scale);

            switch (RxGlobals.PrintSystem.PrinterType)
            {
                case EPrinterType.printer_test_slide_only:
                    for (i = 3; i < InkSuplyGrid.RowDefinitions.Count; i++) InkSuplyGrid.RowDefinitions[i].Height = new GridLength(0);
                    break;

                default:
                    for (i = 3; i < InkSuplyGrid.RowDefinitions.Count; i++) InkSuplyGrid.RowDefinitions[i].Height = new GridLength(1, GridUnitType.Auto);
                    break;
            }
            InkSuplyGrid.RowDefinitions[3].Height = new GridLength(0); // Canister Level
            if (RxGlobals.PrintSystem.HasHeater)  InkSuplyGrid.RowDefinitions[4].Height =  new GridLength(1, GridUnitType.Auto);
            else                                  InkSuplyGrid.RowDefinitions[4].Height =  new GridLength(0);
            _assign_inksupply(RxGlobals.PrintSystem.ColorCnt);
        }

        //--- _assign_inksupply --------------------------------------------------------
        private int  _cnt=0;
        private bool _reverse = false;

        private void _assign_inksupply(int cnt)
        {
            int i;         
            if (RxGlobals.PrintSystem.Reverse!=_reverse || cnt!=_cnt)
            {
                for (i=0; i<cnt; i++) 
                {
                    if (RxGlobals.PrintSystem.Reverse) _InkSupplyView[i].DataContext = _InkSupply.List[cnt-i-1];
                    else                               _InkSupplyView[i].DataContext = _InkSupply.List[i];
                }
            }
            _cnt     = cnt;
            _reverse = RxGlobals.PrintSystem.Reverse;
        }

        //--- show_inkSupplies --------------------------------------
        public void show_inkSupplies(int cnt)
        {
            _assign_inksupply(cnt);
            for (int i=0; i<InkSupplyStack.Children.Count; i++)
            {
                _InkSupplyView[i].Visibility = (i<cnt)? Visibility.Visible : Visibility.Collapsed;
            }
        }

    }
}
