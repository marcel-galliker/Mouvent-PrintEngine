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
    public partial class PrintHeadGrid : UserControl
    {
        private List<PrintHeadView>        _PrintHeadView = new List<PrintHeadView>();

        //--- Constructor ---------------------------
        public PrintHeadGrid()
        {
            InitializeComponent();
            RxGlobals.User.PropertyChanged += User_PropertyChanged;
            User_PropertyChanged(null, null);
        }

        //--- User_PropertyChanged --------------------------------------
        private void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            Visibility visibility  =  (RxGlobals.User.UserType >= EUserType.usr_service) ? Visibility.Visible : Visibility.Collapsed; 
            ServiceGrid.Visibility = visibility;
        }

        //--- _assign_inksupply --------------------------------------------------------
        private void _assign_inksupply(int cnt)
        {
            int i, no;
            if (RxGlobals.PrintSystem.HeadCnt!=0)
            {
                for (i=0;i<PrintHeadStack.Children.Count;i++)
                {
                    try
                    {
                        no = RxGlobals.PrintSystem.IS_Order[i/RxGlobals.PrintSystem.HeadCnt] * RxGlobals.PrintSystem.HeadCnt;
                        if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_TX802)
                            no += RxGlobals.PrintSystem.HeadCnt-1-i%RxGlobals.PrintSystem.HeadCnt;
                        else
                            no += i%RxGlobals.PrintSystem.HeadCnt;
                        _PrintHeadView[i].DataContext = RxGlobals.HeadStat.List[no];
                        _PrintHeadView[i].No = no;
                    }
                    catch(Exception ex)
                    {
                    }
                }
            }
        }

        //--- show_items --------------------------------------
        public void show_items(int cnt)
        {     
            int i, no;      
            RxGlobals.HeadStat.SetItemCount(cnt);
            for (i=PrintHeadStack.Children.Count; i<cnt; i++)
            { 
                _PrintHeadView.Add(new PrintHeadView(i));
                _PrintHeadView[i].DataContext = RxGlobals.HeadStat.List[i];
                PrintHeadStack.Children.Add(_PrintHeadView[i]);
            }
            _assign_inksupply(cnt);
            for (i=0; i<PrintHeadStack.Children.Count; i++)
            {
                no=(int)(i/RxGlobals.PrintSystem.HeadCnt);
                no= RxGlobals.PrintSystem.IS_Order[no];
                _PrintHeadView[i].Visibility = (i<cnt && (RxGlobals.PrintSystem.AllInkSupplies || no==RxGlobals.PrintSystem.CheckedInkSupply))? Visibility.Visible : Visibility.Collapsed;
            }
            Grid.RowDefinitions[1].Height = new GridLength(25/RxGlobals.Screen.Scale);
        }
    }
}
