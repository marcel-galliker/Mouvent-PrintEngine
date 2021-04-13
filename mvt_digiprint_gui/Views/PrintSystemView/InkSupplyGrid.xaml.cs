using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.ComponentModel;
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
    public partial class InkSupplyGrid : UserControl, INotifyPropertyChanged
    {
        private InkSupplyList               _InkSupply     = RxGlobals.InkSupply;
        private List<InkSupplyView>         _InkSupplyView = new List<InkSupplyView>();

        public event PropertyChangedEventHandler PropertyChanged;
        private void OnPropertyChanged(string name)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged.Invoke(this, new PropertyChangedEventArgs(name));
            }
        }

        private bool _HasSideSelection;
        public bool HasSideSelection
        {
            get { return _HasSideSelection; }
            set { _HasSideSelection = value; OnPropertyChanged("HasSideSelection"); }
        }

       
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

            show_inkSupplies(RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.InkCylindersPerColor);
            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
            RxGlobals.User.PropertyChanged += User_PropertyChanged;
            User_PropertyChanged(null, null);
        }

        //--- User_PropertyChanged --------------------------------------
        private void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            Visibility visibility  =  (RxGlobals.User.UserType >= EUserType.usr_maintenance) ? Visibility.Visible : Visibility.Collapsed; 
            ServiceGrid.Visibility = visibility;
        }

        //--- PrintSystem_PropertyChanged ----------------------------------
        private void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("PrinterType")) _printertype_changed();
        //    if (e.PropertyName.Equals("Reverse")) _assign_inksupply(RxGlobals.PrintSystem.ColorCnt);
        }

        //--- _printertype_changed -----------------------------------------
        private void _printertype_changed()
        {
            int i = 0;
            TX_Side.Visibility = RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_DP803? Visibility.Visible : Visibility.Collapsed;
            HasSideSelection = RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_DP803;

            InkSuplyGrid.RowDefinitions[3].Height = new GridLength(25/RxGlobals.Screen.Scale);

            switch (RxGlobals.PrintSystem.PrinterType)
            {
                case EPrinterType.printer_test_slide_only:
                    for (i = 5; i < InkSuplyGrid.RowDefinitions.Count; i++) InkSuplyGrid.RowDefinitions[i].Height = new GridLength(0);
                    break;

                default:
                    for (i = 5; i < InkSuplyGrid.RowDefinitions.Count; i++) InkSuplyGrid.RowDefinitions[i].Height = new GridLength(1, GridUnitType.Auto);
                    break;
            }


            // InkSuplyGrid.RowDefinitions[5].Height = new GridLength(0); // Canister Level
            
            if (RxGlobals.PrintSystem.HasHeater)  InkSuplyGrid.RowDefinitions[5].Height =  new GridLength(1, GridUnitType.Auto);
            else                                  InkSuplyGrid.RowDefinitions[5].Height =  new GridLength(0);
            _assign_inksupply(RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.InkCylindersPerColor);
        }

        //--- _assign_inksupply --------------------------------------------------------
        private void _assign_inksupply(int cnt)
        {
            int i;
            
            for (i=0; i<cnt; i++)
            {
                if (RxGlobals.PrintSystem.IS_Order!=null && i < RxGlobals.PrintSystem.IS_Order.Length)
                {
                    if (RxGlobals.PrintSystem.IS_Order[i] < _InkSupply.List.Count)
                    {
                        _InkSupplyView[i].DataContext = _InkSupply.List[RxGlobals.PrintSystem.IS_Order[i]];
                    }
                }
                else
                {
                    // configuration missmatch!! IS_Order.Length < cnt!
                    break;
                }
            }
            
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
