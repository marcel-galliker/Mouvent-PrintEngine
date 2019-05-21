using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
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
using System.Xml;

namespace RX_DigiPrint.Views.CleafView
{
    /// <summary>
    /// Interaction logic for CleafOrderView.xaml
    /// </summary>
    public partial class CleafOrderView : UserControl
    {
        public CleafOrderView()
        {
            InitializeComponent();

            DataContext = RxGlobals.CleafOrder;
            Flexo1.DataContext = RxGlobals.CleafOrder.flexo[0];
            Flexo2.DataContext = RxGlobals.CleafOrder.flexo[1];
            Flexo3.DataContext = RxGlobals.CleafOrder.flexo[2];
            Flexo4.DataContext = RxGlobals.CleafOrder.flexo[3];

            RxGlobals.CleafOrder.PropertyChanged += CleafOrder_PropertyChanged;
        }

        //--- UserControl_IsVisibleChanged -------------------
        private void UserControl_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue) 
            {
                Operator_DropDownOpened(null, null);
                _update_progress();
            }
        }

        //--- CleafOrder_PropertyChanged -------------------------------------
        private void CleafOrder_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Produced"))  _update_progress();
            if (e.PropertyName.Equals("Waste"))     _update_progress();
        }

        //--- _update_progress ----------------------------
        private void _update_progress()
        {
            if (RxGlobals.CleafOrder.Length==0)
            {
                ProgBar.Value = 0;
                ProgStr.Text="";
                WasteBar.Value = 0;
                WasteStr.Text="";
            }
            else
            { 
                int progress;
                progress=100*RxGlobals.CleafOrder.Produced/RxGlobals.CleafOrder.Length;
                ProgBar.Value = progress;
                ProgStr.Text  = string.Format("{0}m /{1}m ({2}%)", RxGlobals.CleafOrder.Produced, RxGlobals.CleafOrder.Length, progress);
                
                progress=100*RxGlobals.CleafOrder.Waste/RxGlobals.CleafOrder.Length;
                WasteBar.Value = progress;
                WasteStr.Text  = string.Format("{0}m /{1}m ({2}%)", RxGlobals.CleafOrder.Waste, RxGlobals.CleafOrder.Length, progress);
            }
            try
            {
                _set_ink(Ink0, RxGlobals.InkSupply.List[0].InkType, RxGlobals.CleafOrder.ink_ml[0]);
                _set_ink(Ink1, RxGlobals.InkSupply.List[1].InkType, RxGlobals.CleafOrder.ink_ml[1]);
                _set_ink(Ink2, RxGlobals.InkSupply.List[2].InkType, RxGlobals.CleafOrder.ink_ml[2]);
                _set_ink(Ink3, RxGlobals.InkSupply.List[3].InkType, RxGlobals.CleafOrder.ink_ml[3]);
            }
            catch(Exception)
            { 
            }
        }

        //--- _set_ink ---------------------------------
        private void _set_ink(TextBox ctrl, InkType ink, int volume)
        {
            if (ink==null)
            {
                ctrl.Background = Brushes.Transparent;
                ctrl.Foreground = Brushes.Black;
            }
            else
            { 
                ctrl.Background = new SolidColorBrush(ink.Color);
                ctrl.Foreground = new SolidColorBrush(ink.ColorFG);
            }
            ctrl.Text = string.Format("{0} ml", volume);
        }

        //--- Order_Clicked -----------------------------------
        private void Order_Clicked(object sender, RoutedEventArgs e)
        {
            Open.Visibility = Visibility.Visible;
        }

        //--- Operator_DropDownOpened --------------------------------
        private void Operator_DropDownOpened(object sender, EventArgs e)
        {
            ObservableCollection<string> operators = new ObservableCollection<string>();
            try
            {
                XmlTextReader xml = new XmlTextReader(CleafOrder.OrdersDir+"\\operators.xml");
                while(xml.Read())
                {
                    if (xml.NodeType==XmlNodeType.Element && xml.Name.Equals("Operator"))
                        operators.Add(xml.GetAttribute("Name"));
                }
            }
            catch(Exception)
            { }
            CB_Operator.ItemsSource = operators;
        }

        //--- Operator_SelectionChanged -------------------------------------------------
        private void Operator_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_CO_SET_OPERATOR, e.AddedItems[0] as string);
        }
        
    }
}
