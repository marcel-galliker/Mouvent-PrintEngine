using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.PrintSystemView
{
    public partial class PrintHeadGrid : UserControl
    {
        private List<PrintHeadView> _PrintHeadView = new List<PrintHeadView>();

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
            Visibility visibility = (RxGlobals.User.UserType >= EUserType.usr_service) ? Visibility.Visible : Visibility.Collapsed;
            ServiceGrid.Visibility = visibility;
        }

        //--- _assign_inksupply --------------------------------------------------------
        private void _assign_inksupply(int cnt)
        {
            int i, no;
            if (RxGlobals.PrintSystem.IS_Order == null)
            {
                //not ready yet
                return;
            }

            if (RxGlobals.PrintSystem.HeadsPerColor != 0)
            {
                for (i = 0; i < PrintHeadStack.Children.Count; i++)
                {
                    try
                    {
                        no = RxGlobals.PrintSystem.IS_Order[i / RxGlobals.PrintSystem.HeadsPerColor] * RxGlobals.PrintSystem.HeadsPerColor;
                        if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_TX802)
                            no += RxGlobals.PrintSystem.HeadsPerColor - 1 - i % RxGlobals.PrintSystem.HeadsPerColor;
                        else
                            no += i % RxGlobals.PrintSystem.HeadsPerColor;
                        _PrintHeadView[i].DataContext = RxGlobals.HeadStat.List[no];
                        _PrintHeadView[i].No = no;
                    }
                    catch (Exception ex)
                    {
                        // todo
                        // crashed in debug mode!
                        Console.WriteLine(ex.Message);
                    }
                }
            }
        }

        //--- show_items --------------------------------------
        public void show_items(int cnt)
        {
            int i, no;
            if (RxGlobals.PrintSystem.IS_Order == null) return; // not ready yet

            RxGlobals.HeadStat.SetItemCount(cnt);
            for (i = PrintHeadStack.Children.Count; i < cnt; i++)
            {
                _PrintHeadView.Add(new PrintHeadView(i));
                    _PrintHeadView[i].DataContext = RxGlobals.HeadStat.List[i];
                PrintHeadStack.Children.Add(_PrintHeadView[i]);
            }
            _assign_inksupply(cnt);
            for (i = 0; i < PrintHeadStack.Children.Count; i++)
                    {
                no = (int)(i / RxGlobals.PrintSystem.HeadsPerColor);
                no = RxGlobals.PrintSystem.IS_Order[no];
                _PrintHeadView[i].Visibility = (i < cnt && (RxGlobals.PrintSystem.AllInkSupplies || no == RxGlobals.PrintSystem.CheckedInkSupply)) ? Visibility.Visible : Visibility.Collapsed;
                    }
            Grid.RowDefinitions[1].Height = new GridLength(25 / RxGlobals.Screen.Scale);
                }
            }
}


