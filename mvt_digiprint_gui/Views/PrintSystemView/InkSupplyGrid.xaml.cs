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

            show_inkSupplies(RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.InkCylindersPerColor);
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
        //    if (e.PropertyName.Equals("Reverse")) _assign_inksupply(RxGlobals.PrintSystem.ColorCnt);
        }

        //--- _printertype_changed -----------------------------------------
        private void _printertype_changed()
        {
            int i;
            bool first=true;

            for (i=0; i<_InkSupplyView.Count; i++)
			{
                if (_InkSupplyView[i]!=null) 
                {
                    _InkSupplyView[i].PrinterType_Changed();
                    if (first) _InkSupplyView[i].LayoutUpdated = rows_changed;
                    first=false;
                }
			}

            _assign_inksupply(RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.InkCylindersPerColor);
        }

		//--- rows_changed --------------------------------------
        private void rows_changed()
		{
            for (int i=0; i<_InkSupplyView.Count; i++)
			{
                if (_InkSupplyView[i]!=null) 
                {
                    RowDefinitionCollection rows = _InkSupplyView[i].GetRowDefinitions();
                    for (int r=0; r<rows.Count; r++)
			        {
                        InkSuplyGrid.RowDefinitions[r].Height = new GridLength(rows[r].ActualHeight);
			        }
                    if (rows[5].Height.Value==0) InkSuplyGrid_5_0.Visibility = InkSuplyGrid_5_1.Visibility = Visibility.Collapsed;
                    else                         InkSuplyGrid_5_0.Visibility = InkSuplyGrid_5_1.Visibility = Visibility.Visible;
                    return;
                }
            }
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
