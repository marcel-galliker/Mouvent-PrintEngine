using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.UserControls;
using System.Windows;
using System.Windows.Controls;


namespace RX_DigiPrint.Views.PrintQueueView
{
    /// <summary>
    /// Interaction logic for FileSettingsTest.xaml
    /// </summary>
    public partial class FileSettingsTest : UserControl
    {
        static PrintQueueItem _Item;

        //--- Constructor -----------------------------------
        public FileSettingsTest()
        {
            bool init = (_Item==null);

            InitializeComponent();

            if (init) _Item = new PrintQueueItem(){TestImage=ETestImage.jets, Copies=1, LengthUnit=EPQLengthUnit.copies, Dots="S", DropSizes=1};
            this.DataContext = _Item;
            CB_TestImage.ItemsSource = new EN_TestImageList();
            CB_ScanMode.ItemsSource  = new EN_ScanModeList();
            CB_DropSize.ItemsSource  = new EN_DropSize();

            SpeedUnit.Text  = new CUnit("m/min").Name;
            MarginUnit.Text = new CUnit("mm").Name;

            CB_ScanMode.Visibility = CT_ScanMode.Visibility = RxGlobals.PrintSystem.IsScanning? Visibility.Visible:Visibility.Collapsed;

            if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_test_slide
            || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide_only)
            {
                SpeedRow.Height = new GridLength(0);
                ScanRow.Height  = new GridLength(0);
                _Item.PageMargin     = 0;
            }
            else if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_test_table)
            {
                CB_Speed.ItemsSource     = new EN_SpeedTTList();
                CB_ScanMode.ItemsSource  = new EN_ScanModeList();
                 
                if (init)
                {
                    _Item.Speed          = EN_SpeedTTList.DefaultValue;
                    _Item.PageMargin     = 0;
                }
            }
            else if (!RxGlobals.PrintSystem.IsScanning)
            {
                CB_Speed.ItemsSource     = RxGlobals.PrintSystem.SpeedList(1, 2000);
                if (init)
                {
                    MarginRow.Height     = new GridLength(0);
                    _Item.Speed          = EN_SpeedList.DefaultValue;
                    _Item.PageMargin     = 0;
                }
            }
            else
            {
                CB_Speed.ItemsSource     = RxGlobals.PrintSystem.SpeedList(1, 2000);
                if (init)
                {
                    _Item.Speed          = EN_SpeedList.DefaultValue;
                    _Item.PageMargin     = 200;
                }
            }
       }

        //--- Print_Clicked --------------------------------------------------------------------
        private void Print_Clicked(object sender, RoutedEventArgs e)
        {
            if (!RxGlobals.PrinterStatus.AllInkSupliesOn)
            {
                if (RxMessageBox.YesNo("Print System", "Some ink supplies are OFF. Switch them ON.",  MessageBoxImage.Question, true))
                {
                    TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
                    msg.no       = -1;
                    msg.ctrlMode = EFluidCtrlMode.ctrl_print;

                    RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
                }
            }

            if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf)
            {
                int mode = Rx.StrToInt32(RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "PAR_UV_LAMP_3_MODE"));
                if (mode==0)
                {
                    if (!RxMessageBox.YesNo("UV Lamp", "The UV Lamp at printer is NOT READY.\n\nStart Printing?",  MessageBoxImage.Question, false))
                        return;
                }
            }

            _Item.ScanLength = _Item.Copies;
            switch(_Item.TestDotSize)
            {
            case 2: _Item.Dots="M"; break;
            case 3: _Item.Dots="L"; break;
            default: _Item.Dots="S"; break;
            }
            if (DataContext!=null)
                (DataContext as PrintQueueItem).SendMsg(TcpIp.CMD_TEST_START);

            var myWindow = Window.GetWindow(this);
            myWindow.Close();
        }
    }
}
