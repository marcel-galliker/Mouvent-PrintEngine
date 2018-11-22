using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Windows;
using System.Windows.Controls;


namespace RX_DigiPrint.Views.PrintQueueView
{
    /// <summary>
    /// Interaction logic for FileSettings.xaml
    /// </summary>
    public partial class FileSettingsScan : UserControl
    {
        //--- Constructor -----------------------------------
        public FileSettingsScan()
        {
            InitializeComponent();          
            Visibility = RxGlobals.PrintSystem.IsScanning ? Visibility.Visible : Visibility.Collapsed;
            Print_Settings.Visibility = Visibility.Visible;
            ShowButtonGrid.Visibility = Visibility.Collapsed;
            Number_Settings.Visibility= Visibility.Collapsed;

            CB_ScanMode.ItemsSource     = new EN_ScanModeList(); 
            CB_Passes.ItemsSource       = new EN_PassesList();
            CB_Orientation.ItemsSource  = new EN_OrientationList();
        }  

        //--- UserControl_DataContextChanged ----------------------------------
        private void UserControl_DataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            PrintQueueItem item = DataContext as PrintQueueItem;
            if (item!=null) CB_Speed.ItemsSource = RxGlobals.PrintSystem.SpeedList(item.LargestDot);
        }

        //--- Property ShowSaveButton -----------------------------------------------
        public bool ShowSaveButton
        {
            set { ShowButtonGrid.Visibility = value? Visibility.Visible:Visibility.Collapsed;}
        }

        //--- Save_Clicked --------------------------------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            PrintQueueItem item = DataContext as PrintQueueItem;
            if (item!=null)
            {
                item.PrintGoMode = TcpIp.EPrintGoMode.PG_MODE_GAP;
                item.SendMsg(TcpIp.CMD_SET_PRINT_QUEUE);
            }
        }

        //--- Cancel_Clicked --------------------------------------------------------------------
        private void Cancel_Clicked(object sender, RoutedEventArgs e)
        {
            PrintQueueItem item = DataContext as PrintQueueItem;
            if (item!=null) item.SendMsg(TcpIp.CMD_GET_PRINT_QUEUE_ITM);
        }
        
        //--- Save_Default_Clicked --------------------------------------------------------------------
        private void Save_Default_Clicked(object sender, RoutedEventArgs e)
        {
            PrintQueueItem item = DataContext as PrintQueueItem;
            if (item!=null)
            {
                item.PrintGoMode = TcpIp.EPrintGoMode.PG_MODE_GAP;
                item.SendMsg(TcpIp.CMD_SET_PRINT_QUEUE);
                item.SaveDefaults(false);
            }
        }

        //--- Numbers_Clicked -------------------------------------------------
        private void Numbers_Clicked(object sender, RoutedEventArgs e)
        {
            if (Number_Settings.Visibility==Visibility.Collapsed)
            {
                Number_Settings.Visibility= Visibility.Visible; 
                Print_Settings.Visibility = Visibility.Collapsed;            
            }
            else
            {
                Number_Settings.Visibility= Visibility.Collapsed; 
                Print_Settings.Visibility = Visibility.Visible;            
            }
        }

        //--- PageNumber_Expanded --------------------------------------------
        private void PageNumber_Expanded(object sender, RoutedEventArgs e)
        {
            Number_Settings.Visibility= Visibility.Visible; 
        }

        //--- PageNumber_Collapsed --------------------------------------------
        private void PageNumber_Collapsed(object sender, RoutedEventArgs e)
        {
            Number_Settings.Visibility= Visibility.Collapsed; 
        }

        //--- Checks_Expanded -------------------------------------------------
        private void Checks_Expanded(object sender, RoutedEventArgs e)
        {
        //  Checks_Settings.Visibility= Visibility.Visible; 
        }

        //--- Checks_Collapsed -------------------------------------------------
        private void Checks_Collapsed(object sender, RoutedEventArgs e)
        {
        //  Checks_Settings.Visibility= Visibility.Collapsed; 
        }
    }
}
