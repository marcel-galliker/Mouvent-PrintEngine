using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Diagnostics;
using System.Reflection;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace RX_DigiPrint.Views
{
    public partial class LogView : UserControl
    {
        private bool searching=false;

        public LogView()
        {
            InitializeComponent();
            LogGrid.ItemsSource = RxGlobals.Log.List;
            Scroll.Minimum  = 0;
            Scroll.Maximum  = 0;
            Scroll.Value    = 0;
            Scroll.ViewportSize = 0;
            
            //--- test assembly number -----------------------------------------------
            Assembly ass = Assembly.GetExecutingAssembly();
            FileVersionInfo FVI = FileVersionInfo.GetVersionInfo(ass.Location);
            Console.WriteLine("Product: {0} v ({1})", FVI.ProductName, FVI.ProductVersion);
            Console.WriteLine("File: {0} v ({1})", FVI.FileName, FVI.FileVersion);

            Search_clicked(this, null);

            RxGlobals.Log.List.CollectionChanged += Log_CollectionChanged;

            //--- test publish number -------------------------------------------------
            // needs "System.Deployment" in References
            if (System.Deployment.Application.ApplicationDeployment.IsNetworkDeployed) 
            {
                System.Deployment.Application.ApplicationDeployment ad = System.Deployment.Application.ApplicationDeployment.CurrentDeployment;
                Version  v = ad.CurrentVersion;
                Console.WriteLine("Bublish Version: {0}.{1}.{2}.{3}", v.Major, v.Minor, v.Build, v.Revision);
            }

            //--- test language -------------------------------------------------------
            Console.Write(">>{0}<<", RxGlobals.Language.GetString("Test"));
            Console.WriteLine(">>{0}<<", RxGlobals.Language.GetString("OK"));  
            Console.WriteLine(">>{0}<<", RxGlobals.Language.GetString("Error", "1"));  
            //--- END TESTS ----------------------------------------------------------------
        }

        //--- Log_CollectionChanged ----------------------------------------------
        void Log_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            if (searching && Scroll.Value != RxGlobals.Log.Pos)
            {
                Scroll.Value=RxGlobals.Log.Pos;
            }
        }

        //--- LogGrid_CellControlAttached ---------------------------------------------------------------------------------------------------
        private void LogGrid_CellControlAttached(object sender, Infragistics.Controls.Grids.CellControlAttachedEventArgs e)
        {
            bool alternate = RxGlobals.Log.List[e.Cell.Row.Index].Alternate;
            Infragistics.Controls.Grids.CellControl ctrl=e.Cell.Control as Infragistics.Controls.Grids.CellControl;         
            if (ctrl!=null)
            { 
                switch(RxGlobals.Log.List[e.Cell.Row.Index].LogType)
                {
                case ELogType.eErrWarn:                        
                        if (alternate)  ctrl.Background    = (Brush)FindResource("XamGrid_WRN_Alternate");
                        else            ctrl.Background    = (Brush)FindResource("XamGrid_WRN_Normal");                       
                        ctrl.SetValue(Helpers.RxCellControl.MouseOverBrushProperty, (Brush)FindResource("XamGrid_WRN_MouseOver"));
                        break;

                case ELogType.eErrCont:
                case ELogType.eErrStop:
                case ELogType.eErrAbort:
                        if (alternate)  ctrl.Background    = (Brush)FindResource("XamGrid_ERR_Normal");
                        else            ctrl.Background    = (Brush)FindResource("XamGrid_ERR_Alternate");
                        ctrl.SetValue(Helpers.RxCellControl.MouseOverBrushProperty, (Brush)FindResource("XamGrid_ERR_MouseOver"));
                        break;

                default:
                        if (alternate)  ctrl.Background    = (Brush)FindResource("XamGrid_LOG_Normal");
                        else            ctrl.Background    = (Brush)FindResource("XamGrid_LOG_Alternate");
                        ctrl.SetValue(Helpers.RxCellControl.MouseOverBrushProperty, (Brush)FindResource("XamGrid_LOG_MouseOver"));
                        break;
                }
            }

            Scroll.Maximum = RxGlobals.Log.ItemCount;
        }

        //--- Confirm_clicked ----------------------------------------------------------
        private void Confirm_clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_EVT_CONFIRM);
        }

        //--- LogGrid_SizeChanged ----------------------------------------------------------
        private void LogGrid_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (LogGrid.MinimumRowHeight!=0)
            {
                UInt32 rows;
                rows = (UInt32)(LogGrid.ActualHeight/LogGrid.MinimumRowHeight);
                if (RxScreen.Screen.Surface) rows-=2;
                else rows-=1;

                Scroll.ViewportSize = rows;
                if (rows>2) Scroll.LargeChange  = rows-2;
                else        Scroll.LargeChange  = 1;
                Scroll.SmallChange = 1;
                SendLogRequest("", "", Scroll.Value, rows);
            }
        }

        //--- Scroll_ValueChanged -----------------------------------------------------------------
        private void Scroll_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (searching) searching=false;
            else SendLogRequest("", "", Scroll.Value, Scroll.ViewportSize);
        }

        //--- SendLogRequest -----------------------------------------------------
        private void SendLogRequest(string filepath, string find, double first, double count)
        {
            TcpIp.SLogReqMsg msg = new TcpIp.SLogReqMsg();
            msg.filepath        = filepath;
            msg.find            = find;
            msg.first           = (Int32)first;
            msg.count           = (Int32)count;
            
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_REQ_LOG, ref msg);
        }

        //--- Search_clicked ----------------------------------------------------------
        private void Search_clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            if (SearchButton.IsChecked) 
            {
                SearchText.Visibility   = Visibility.Visible;
                SearchUp.Visibility     = Visibility.Visible;
                SearchDown.Visibility   = Visibility.Visible;
                MoveTop.Visibility      = Visibility.Collapsed;
                MoveBottom.Visibility   = Visibility.Collapsed;

                SearchText.Focus();
            }
            else
            {
                SearchText.Visibility   = Visibility.Collapsed;
                SearchUp.Visibility     = Visibility.Collapsed;
                SearchDown.Visibility   = Visibility.Collapsed;
                MoveTop.Visibility      = Visibility.Visible;
                MoveBottom.Visibility   = Visibility.Visible;
            } 
        }

        //--- SearchUp_clicked ----------------------------------------------------------
        private void SearchUp_clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            SendLogRequest("", SearchText.Text, -(Scroll.Value-1), Scroll.ViewportSize);
            searching = true;
        }

        //--- SearchDown_clicked ----------------------------------------------------------
        private void SearchDown_clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            SendLogRequest("", SearchText.Text, Scroll.Value+1, Scroll.ViewportSize);
            searching = true;
        }

        //--- Export_clicked ----------------------------------------------------------
        private void Export_clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            TcpIp.SLogReqMsg msg = new TcpIp.SLogReqMsg();
            msg.first           = 0;
            msg.count           = 200;
            msg.count           = 2000;
//          msg.count           = 100000;
            
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_EXPORT_LOG, ref msg);
        }

        //--- Top_clicked ----------------------------------------------------------
        private void Top_clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            if (Scroll.Value == 0 ) Scroll.Value =1;
            Scroll.Value = 0;
        }

        //--- Top_clicked ----------------------------------------------------------
        private void PageUp_clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            if (Scroll.Value == 0 ) Scroll.Value =1;
            double val = Scroll.Value-Scroll.LargeChange;
            if (val<0) val=0;
            Scroll.Value = val;
        }

        //--- Top_clicked ----------------------------------------------------------
        private void PageDown_clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            double val = Scroll.Value+Scroll.LargeChange;
            if (val>Scroll.Maximum-Scroll.LargeChange) val=Scroll.Maximum-Scroll.LargeChange;
            Scroll.Value = val;
        }

        //--- Bottom_clicked ----------------------------------------------------------
        private void Bottom_clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            double val = Scroll.Maximum-Scroll.LargeChange;
            if (val>0) Scroll.Value = val;
            else       Scroll.Value = 0;
        }

     }
}
