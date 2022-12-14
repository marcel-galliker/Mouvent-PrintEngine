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
    public partial class FileSettingsWeb : UserControl//, INotifyPropertyChanged
    {
        /*
        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string name)
        {
			PropertyChanged?.Invoke(this,new PropertyChangedEventArgs(name));
		}
        */

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

            if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_LH702 && !RxGlobals.PrintSystem.LH702_simulation) SpeedHeight.Height = new GridLength(0);
            else SpeedHeight.Height = GridLength.Auto;
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
                if (!item.SinglePage && (item.Variable || item.SrcPages > 1))
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
                        return Math.Round(item.StartFrom * unit.Factor * 100) / 100;
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
                        item.StartFrom = value / unit.Factor;
                    }
                    else item.StartFrom = (int)value;
                }
            }
        }
	}
}
