using RX_Common.Source;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
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
using System.Windows.Shapes;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;

namespace RX_DigiPrint.Views.CleafView
{
    /// <summary>
    /// Interaction logic for CleafOpenOrder.xaml
    /// </summary>
    public partial class CleafOpenOrder : UserControl
    {
        private ObservableCollection<CleafOrder> _Orders;
        private ObservableCollection<string>     _OrderDir;

        public CleafOpenOrder()
        {
            InitializeComponent();
        }
        
        //--- _UpdateOrdersList -------------------------------------
        private void _UpdateOrdersList()
        {
            if (!RxNetUse.ShareConnected(CleafOrder.OrdersDir))
            {
                RxNetUse.DeleteShare(CleafOrder.OrdersDir);
                RxNetUse.OpenShare(CleafOrder.OrdersDir, null, null);
            }
            if (RxNetUse.ShareConnected(CleafOrder.OrdersDir))
            {
                var dirs  = Directory.GetDirectories(CleafOrder.OrdersDir);
                Console.WriteLine("Directory {0}", CleafOrder.OrdersDir);
                _Orders = new ObservableCollection<CleafOrder>();
                _OrderDir = new ObservableCollection<string>();
                foreach (string dirname in dirs)
                {
                    try
                    {                        
                        CleafOrder order =  new CleafOrder();
                        order.ReadFileInDir(dirname);
                        if(order.Id!=null)
                        {
                            _OrderDir.Add(dirname);
                            _Orders.Add(order);
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.Message);
                    }
                }
                {
                    CleafOrder order =  new CleafOrder(){Id="unused", Code=""};
                    _OrderDir.Add("");
                    _Orders.Add(order);
                }
           }
           Orders.ItemsSource = _Orders;
        }

        //--- Yes_Clicked -----------------------------------------------
        private void Yes_Clicked(object sender, RoutedEventArgs e)
        {
            try
            {
                int row=Orders.ActiveCell.Row.Index;
                string[] part = _OrderDir[row].Split('\\');
                if (part.Count()>4) RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_CO_SET_ORDER, part[4]);
                else                RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_CO_SET_ORDER, "");
            }
            catch(Exception)
            {
                RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_CO_SET_ORDER, "");
            }
            Visibility = Visibility.Collapsed;
        }

        //--- No_Clicked -------------------------------------------------
        private void No_Clicked(object sender, RoutedEventArgs e)
        {
            Visibility = Visibility.Collapsed;
        }

        //--- UserControl_IsVisibleChanged ------------------------------------------
        private void UserControl_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue==true) _UpdateOrdersList();
        }
    }

}
