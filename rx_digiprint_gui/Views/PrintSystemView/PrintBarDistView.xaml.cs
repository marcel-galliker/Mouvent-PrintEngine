using RX_DigiPrint.Models;
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
    /// Interaction logic for PrintBarDistView.xaml
    /// </summary>
    public partial class PrintBarDistView : UserControl
    {
//        private int _headCnt;

        public PrintBarDistView()
        {
            InitializeComponent();
            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
        }

        //--- PrintSystem_PropertyChanged ---------------------------------------
        void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
             if (e.PropertyName.Equals("CheckedInkSupply") || e.PropertyName.Equals("AllInkSupplies"))
             {
                _ShowControls();
             }
             if (e.PropertyName.Equals("ColorCnt") || e.PropertyName.Equals("HeadCnt"))
             {
                _CreateControls();
                _ShowControls();
             }
        }

        //--- _CreateControls ---------------------------------------------------
        private void _CreateControls()
        {
            int i, n;
            int headCnt = RxGlobals.PrintSystem.ColorCnt*(int)RxGlobals.PrintSystem.HeadCnt;
            n = (headCnt+3)/4;
            if (n!=Panel.Children.Count)
            {
                Panel.Children.RemoveRange(0, Panel.Children.Count);
                for (i=0; i<n; i++)
                {                
                    PrintHeadDistView view = new PrintHeadDistView(i, (int)RxGlobals.PrintSystem.HeadCnt);
                    Panel.Children.Add(view);
                }
            }
        }

        //--- _ShowControls -----------------------------------------------------
        private void _ShowControls()
        {
            int i, inkSupply;
            int headCnt=RxGlobals.PrintSystem.HeadCnt;
            if (headCnt==0) return;
            for (i=0; i<Panel.Children.Count; i++)
            {
                inkSupply = (int)((i*4)/RxGlobals.PrintSystem.HeadCnt);
                if (RxGlobals.PrintSystem.Reverse) inkSupply = RxGlobals.PrintSystem.ColorCnt-1-inkSupply;
                if (RxGlobals.PrintSystem.AllInkSupplies) 
                    Panel.Children[i].Visibility = (RxGlobals.PrintSystem.AllInkSupplies || (inkSupply/headCnt)==(RxGlobals.PrintSystem.CheckedInkSupply/headCnt)) ? Visibility.Visible : Visibility.Collapsed;
                else
                    Panel.Children[i].Visibility = (RxGlobals.PrintSystem.AllInkSupplies || inkSupply==RxGlobals.PrintSystem.CheckedInkSupply) ? Visibility.Visible : Visibility.Collapsed;
            }
        }
    }
}
