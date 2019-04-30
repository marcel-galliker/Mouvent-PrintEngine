using Infragistics.Windows.Editors;
using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;


namespace RX_DigiPrint.Views.PrintQueueView
{
    /// <summary>
    /// Interaction logic for FileSettings.xaml
    /// </summary>
    public partial class FileSettingsWeb : UserControl
    {
        //--- Constructor -----------------------------------
        public FileSettingsWeb()
        {
            InitializeComponent();
            Visibility = RxGlobals.PrintSystem.IsScanning ? Visibility.Collapsed : Visibility.Visible;
            PrintQueueItem item = DataContext as PrintQueueItem;
            CB_PrintGoMode.ItemsSource  = new EN_PgModeList();
            LengthBox.ShowRolls = (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf);
        }
       
        //--- UserControl_DataContextChanged ----------------------------------
        private void UserControl_DataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            PrintQueueItem item = DataContext as PrintQueueItem;
            if (item!=null)
            {
                CB_Speed.ItemsSource = RxGlobals.PrintSystem.SpeedList(item.LargestDot);
                if (item.Variable || item.SrcPages>1)
                { 
                    Length_Settings.Visibility = Visibility.Collapsed;
                    Page_Settings.Visibility   = StartPageTxt.Visibility = StartPageNum.Visibility = Visibility.Visible;
                }
                else
                {
                    Length_Settings.Visibility = Visibility.Visible;
                    Page_Settings.Visibility   = StartPageTxt.Visibility = StartPageNum.Visibility = Visibility.Collapsed;
                }
            }
        }

        //--- Save_Click --------------------------------------------------------------------
        private void Save_Click(object sender, RoutedEventArgs e)
        {
            PrintQueueItem item = DataContext as PrintQueueItem;
            if (item!=null)
            {
                if (Page_Settings.Visibility==Visibility.Visible) item.ScanLength = 0;
                item.SendMsg(TcpIp.CMD_SET_PRINT_QUEUE);
            }
        }

        //--- Cancel_Click --------------------------------------------------------------------
        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            PrintQueueItem item = DataContext as PrintQueueItem;
            if (item!=null) item.SendMsg(TcpIp.CMD_GET_PRINT_QUEUE_ITM);
        }
        
        //--- Save_Default_Click --------------------------------------------------------------------
        private void Save_Default_Click(object sender, RoutedEventArgs e)
        {
            PrintQueueItem item = DataContext as PrintQueueItem;
            if (item!=null)
            {
                if (Page_Settings.Visibility==Visibility.Visible) item.ScanLength = 0;
                item.SendMsg(TcpIp.CMD_SET_PRINT_QUEUE);
                item.SaveDefaults(false);
            }
        }
    }
}
