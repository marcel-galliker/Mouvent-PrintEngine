using RX_Common;
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
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.PrintSystemView
{
    /// <summary>
    /// Interaction logic for HeadAdjusting.xaml
    /// </summary>
    public partial class HeadAdjustment : Window
    {
        private int _inkSupplyNo;
        private int _headNo;
        RxScreen _Screen = new RxScreen();

        public HeadAdjustment(Control parent, int inkSupplyNo, int headNo)
        {
            InitializeComponent();

            Left=0;
            Top=0;
            Width=_Screen.Width;
            Height=_Screen.Height;

            Point pt = parent.PointToScreen(new Point(0, parent.ActualHeight));

            if (_Screen.Surface) 
            {
                Back.BorderThickness = new Thickness(pt.X*_Screen.Scale, pt.Y*_Screen.Scale-parent.ActualHeight, (_Screen.Width-pt.X)*_Screen.Scale-parent.ActualWidth*0.85, (_Screen.Height-pt.Y)*0.52);
                double h=Dialog.ActualHeight;
                Canvas.SetLeft(Dialog, pt.X*_Screen.Scale);           
                Canvas.SetTop (Dialog, pt.Y*_Screen.Scale);
            }
            else
            {
                Back.BorderThickness = new Thickness(pt.X, pt.Y-parent.ActualHeight, _Screen.Width-pt.X-parent.ActualWidth, _Screen.Height-pt.Y);
                Canvas.SetLeft(Dialog, pt.X);           
                Canvas.SetTop(Dialog,  pt.Y);
            }

            _inkSupplyNo = inkSupplyNo;
            _headNo      = headNo;

            DataContext = RxGlobals.HeadAdjustment;
        }

        //--- Dialog_SizeChanged ---------------------------------------------
        private void Dialog_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            Canvas.SetTop (Dialog, Back.BorderThickness.Top - Dialog.ActualHeight-2);
        }

        //---Adjust_Clicked ------------------------------------------------------
        private void Adjust_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.HeadAdjustment.Adjust();
        }

        //---Cancel_Clicked ---------------------------------------------------------
        private void Cancel_Clicked(object sender, RoutedEventArgs e)
        {            
            DialogResult = false;
        }

        //---Done_Clicked ---------------------------------------------------------
        private void Done_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.HeadAdjustment.Reset();
            DialogResult = true;
        }

        //--- Help_Clicked ------------------------------------------------
        private void Help_Clicked(object sender, RoutedEventArgs e)
        {
            AdjustmentHelp dlg = new AdjustmentHelp();
            dlg.Show();
        }
    }
}
