using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;


namespace RX_DigiPrint.Views.PrintQueueView
{
    /// <summary>
    /// Interaction logic for FileSettings.xaml
    /// </summary>
    public partial class FileSettingsWeb : UserControl, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        //--- Constructor -----------------------------------
        public FileSettingsWeb()
        {
            InitializeComponent();
            Visibility = RxGlobals.PrintSystem.IsScanning ? Visibility.Collapsed : Visibility.Visible;
            
            PrintQueueItem item = DataContext as PrintQueueItem;
            CB_PrintGoMode.ItemsSource  = new EN_PgModeList();
            RxGlobals.Settings.PropertyChanged += Settings_PropertyChanged;
            LengthBox.ShowRolls = (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf);

            NumBox_StartFrom.DataContext = this;
        }

        void Settings_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Units"))
            {
                var datacontext = DataContext;
                DataContext = null;
                DataContext = datacontext;
            }
        }
       
        //--- UserControl_DataContextChanged ----------------------------------
        private void UserControl_DataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            PrintQueueItem item = DataContext as PrintQueueItem;
            if (item != null)
            {
                LengthUnit.Content = new CUnit("m").Name;
                SpeedUnit.Text = new CUnit("m/min").Name;
                MarginUnit.Text = DistUnit.Text = new CUnit("mm").Name;

                CB_Speed.ItemsSource = RxGlobals.PrintSystem.SpeedList(item.LargestDot, item.SrcHeight);
                if (item.Variable || item.SrcPages > 1)
                {
                    Length_Settings.Visibility = Visibility.Collapsed;
                    Page_Settings.Visibility = StartPageTxt.Visibility = StartPageNum.Visibility = Visibility.Visible;
                    item._hasPageSettings = true;
                }
                else
                {
                    Length_Settings.Visibility = Visibility.Visible;
                    Page_Settings.Visibility = StartPageTxt.Visibility = StartPageNum.Visibility = Visibility.Collapsed;
                    item._hasPageSettings = false;
                }
                Settings.Visibility = Visibility.Visible;
            }
            else
            {
                DataContext = null;
                Settings.Visibility = Visibility.Hidden;
            }
        }

        //--- Property StartFrom ---------------------------------------
        public double StartFrom
        {
            get
            {
                PrintQueueItem item = DataContext as PrintQueueItem;
                if (item != null)
                {
                    if (item.LengthUnitMM)
                    {
                        CUnit unit = new CUnit("m");
                        return Math.Round(item.StartFrom * unit.Factor);
                    }
                    return item.StartFrom;
                }
                return 0;
            }
            set
            {
                PrintQueueItem item = DataContext as PrintQueueItem;
                if (item != null)
                {
                    if (item.LengthUnitMM)
                    {
                        CUnit unit = new CUnit("m");
                        item.StartFrom = (int)Math.Round(value / unit.Factor);
                    }
                    else item.StartFrom = (int)value;
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
                item.SaveDefaults();
            }
        }

        //--- Cancel_Click --------------------------------------------------------------------
        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            PrintQueueItem item = DataContext as PrintQueueItem;
            if (item!=null) item.SendMsg(TcpIp.CMD_GET_PRINT_QUEUE_ITM);
        }

		private void UserControl_IsVisibleChanged(object sender,DependencyPropertyChangedEventArgs e)
		{
            if ((bool)e.NewValue)
                Specials_LB702.Visibility = (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_LB702_UV) ? Visibility.Visible : Visibility.Collapsed;
		}
	}
}
