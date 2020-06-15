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
            Visibility visibility  =  (RxGlobals.User.UserType >= EUserType.usr_service) ? Visibility.Visible : Visibility.Collapsed; 
            ServiceGrid.Visibility = visibility;
        }

        //--- _assign_inksupply --------------------------------------------------------
        private void _assign_inksupply(int cnt)
        {
            int i, no;
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


        public void show_items(int cnt)
        {
            // add items to Print head stack if there are not enough:
            int totalHeadCnt = RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.HeadsPerColor;

            RxGlobals.HeadStat.SetItemCount(totalHeadCnt);
            for (int i = PrintHeadStack.Children.Count; i < cnt; i++)
            {
                _PrintHeadView.Add(new PrintHeadView(i));
                try
                {
                    _PrintHeadView[i].DataContext = RxGlobals.HeadStat.List[i];
                }
                catch (Exception ex)
                {
                    // RxGlobals.HeadStat.List does not contain enough elements!
                    Console.WriteLine(ex.Message);
                }
                PrintHeadStack.Children.Add(_PrintHeadView[i]);
            }

            // Assign data context:
            if (RxGlobals.PrintSystem.AllInkSupplies)
            {
                for (int i = 0; i < totalHeadCnt; i++)
                {
                    int no = i;
                    if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_TX802)
                    {
                        no = totalHeadCnt - 1 - i;
                    }
                    try
                    {
                        _PrintHeadView[i].DataContext = RxGlobals.HeadStat.List[no];
                        _PrintHeadView[i].No = no;
                        _PrintHeadView[i].Visibility = System.Windows.Visibility.Visible;
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.Message);
                    }
                }
            }
            else
            {
                int globalInkCylinderIndex = RxGlobals.PrintSystem.CheckedInkSupply;
                int firstPrintHeadIndex = globalInkCylinderIndex * RxGlobals.PrintSystem.HeadsPerInkCylinder;
                int lastPrintHeadIndex  = firstPrintHeadIndex + RxGlobals.PrintSystem.HeadsPerInkCylinder - 1;
                int no = 0;
                for (int i = 0; i < RxGlobals.PrintSystem.HeadsPerInkCylinder; i++)
                {
                    no = firstPrintHeadIndex + i;
                    if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_TX802)
                    {
                        int diff = no - firstPrintHeadIndex;
                        no = lastPrintHeadIndex - diff;
                        // no = ((RxGlobals.PrintSystem.PrintHeadsPerInkCylinderCnt - 1) - (firstPrintHeadIndex + i)) % RxGlobals.PrintSystem.PrintHeadsPerInkCylinderCnt;
                        // no += globalInkCylinderIndex * RxGlobals.PrintSystem.PrintHeadsPerInkCylinderCnt;
                    }

                    try
                    {
                        _PrintHeadView[i].DataContext = RxGlobals.HeadStat.List[no];
                        _PrintHeadView[i].No = no;
                        _PrintHeadView[i].Visibility = System.Windows.Visibility.Visible;
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.Message);
                    }
                }
            }


            // hide all unnecessary items
            for (int i = cnt; i < PrintHeadStack.Children.Count; i++)
            {
                if (i < _PrintHeadView.Count)
                {
                    _PrintHeadView[i].Visibility = System.Windows.Visibility.Collapsed;
                }
            }
        }
    }
}
