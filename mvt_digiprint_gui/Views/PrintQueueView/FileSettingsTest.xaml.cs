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
    /// Interaction logic for FileSettingsTest.xaml
    /// </summary>
    public partial class FileSettingsTest : UserControl
    {
        static PrintQueueItem _Item;

        private bool SpeedIsShown { get; set; }
        private bool ScanModeIsShown { get; set; }
        private bool MarginIsShown { get; set; }


        //--- Constructor -----------------------------------
        public FileSettingsTest()
        {
            bool init = (_Item == null);

            InitializeComponent();

            SpeedIsShown = true;
            ScanModeIsShown = true;
            MarginIsShown = true;

            if (init) _Item = new PrintQueueItem() { TestImage = ETestImage.jets, Copies = 1, LengthUnit = EPQLengthUnit.copies, Dots = "S", DropSizes = 1 };
            this.DataContext = _Item;
            CB_TestImage.ItemsSource = new EN_TestImageList();
            CB_ScanMode.ItemsSource = new EN_ScanModeList();
            CB_DropSize.ItemsSource = new EN_DropSize();

            SpeedUnit.Text = new CUnit("m/min").Name;
            MarginUnit.Text = new CUnit("mm").Name;

            CB_ScanMode.Visibility = CT_ScanMode.Visibility = RxGlobals.PrintSystem.IsScanning ? Visibility.Visible : Visibility.Collapsed;
            ScanModeIsShown = CB_ScanMode.Visibility == Visibility.Visible;

            if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide
            || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide_only)
            {
                SpeedRow.Height = new GridLength(0);
                SpeedIsShown = false;
                ScanRow.Height = new GridLength(0);
                ScanModeIsShown = false;
                _Item.PageMargin = 0;
            }
            else if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_table)
            {
                CB_Speed.ItemsSource = new EN_SpeedTTList();
                CB_ScanMode.ItemsSource = new EN_ScanModeList();

                if (init)
                {
                    _Item.Speed = EN_SpeedTTList.DefaultValue;
                    _Item.PageMargin = 0;
                }
            }
            else if (!RxGlobals.PrintSystem.IsScanning)
            {
                CB_Speed.ItemsSource = RxGlobals.PrintSystem.SpeedList(1, 2000);
                if (init)
                {
                    MarginRow.Height = new GridLength(0);
                    MarginIsShown = false;
                    _Item.Speed = EN_SpeedList.DefaultValue;
                    _Item.PageMargin = 0;
                }
            }
            else
            {
                CB_Speed.ItemsSource = RxGlobals.PrintSystem.SpeedList(1, 2000);
                if (init)
                {
                    _Item.Speed = EN_SpeedList.DefaultValue;
                    _Item.PageMargin = 200;
                }
            }
        }

        //--- Print_Clicked --------------------------------------------------------------------
        private void Print_Clicked(object sender, RoutedEventArgs e)
        {
            if (InkSupply.AnyFlushed()) return;

            if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_cleaf && !(RxGlobals.StepperStatus[0].DripPans_InfeedDOWN && RxGlobals.StepperStatus[0].DripPans_OutfeedDOWN))
            {
                MvtMessageBox.YesNo("Print System", "Drip Pans below the clusters. Move it out before printing", MessageBoxImage.Question, true);
                return;
            }

            if (!RxGlobals.PrinterStatus.AllInkSupliesOn)
            {
                if (MvtMessageBox.YesNo("Print System", "Some ink supplies are OFF. Switch them ON.", MessageBoxImage.Question, true))
                {
                    TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
                    msg.no = -1;
                    msg.ctrlMode = EFluidCtrlMode.ctrl_print;

                    RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
                }
            }

            _Item.ScanLength = _Item.Copies;
            switch (_Item.TestDotSize)
            {
                case 2: _Item.Dots = "M"; break;
                case 3: _Item.Dots = "L"; break;
                default: _Item.Dots = "S"; break;
            }
            if (DataContext != null)
                (DataContext as PrintQueueItem).SendMsg(TcpIp.CMD_TEST_START);

            var myWindow = Window.GetWindow(this);
            myWindow.Close();
        }

        private void MessageCheckBoxChanged(object sender, RoutedEventArgs e)
        {
            CheckBox checkBox = sender as CheckBox;
            if (checkBox == null)
            {
                return;
            }
            bool? isChecked = checkBox.IsChecked;
            if (isChecked.HasValue)
            {
                if ((bool)isChecked)
                {
                    SetTestMessage();
                }
                else
                {
                    _Item.TestMessage = "";
                }
            }
        }

        private void SetTestMessage()
        {
            string testMessage = "";
            if(_Item != null)
            {
                testMessage += "TestImg: " + TestImageToString(_Item.TestImage);
                testMessage += " | DropSize: " + TestDotSizeToString(_Item.TestDotSize);
                if (SpeedIsShown) testMessage += " | Speed: " + _Item.Speed.ToString() + SpeedUnit.Text;
                if(ScanModeIsShown) testMessage += " | Scan Mode: " + ScanModeToString(_Item.ScanMode);
                if(MarginIsShown) testMessage += " | Page Margin: " + _Item.PageMargin.ToString("N2") + MarginUnit.Text;
                
                _Item.TestMessage = testMessage;
            }
        }

        private string ScanModeToString(EScanMode mode)
        {
            string scanModeString = "";
            switch (mode)
            {
                case EScanMode.scan_std:
                    scanModeString = "--->";
                    break;
                case EScanMode.scan_rtl:
                    scanModeString = "<---";
                    break;
                case EScanMode.scan_bidir:
                    scanModeString = "<-->";
                    break;
                default:
                    scanModeString = "?";
                    break;
            }
            return scanModeString;
        }

        private string TestDotSizeToString(int dotSize)
        {
            string dotsSizeString = "";
            switch (dotSize)
            {
                case 2: dotsSizeString = "M"; break;
                case 3: dotsSizeString = "L"; break;
                default: dotsSizeString = "S"; break;
            }
            return dotsSizeString;
        }


        private string TestImageToString(ETestImage testImage)
        {
            string testImageString = "";
            switch (testImage)
            {
                case ETestImage.undef: testImageString = "Undef"; break;
                case ETestImage.angle_overlap: testImageString = "Angle Overlap"; break;
                case ETestImage.angle_separated: testImageString = "Angle Separated"; break;
                case ETestImage.jets: testImageString = "Jets";  break;
                case ETestImage.jet_numbers: testImageString = "Jet Numbers"; break;
                case ETestImage.grid: testImageString = "Grid"; break;
                case ETestImage.encoder: testImageString = "Encoder"; break;
                case ETestImage.scanning: testImageString = "Scanning"; break;
                case ETestImage.fullAlignment: testImageString = "Full Alignment"; break;
                default: testImageString = "Undef";  break;
            }
            return testImageString;
        }

        private void ComboBox_LostFocus(object sender, RoutedEventArgs e)
        {
            bool? isChecked = MessageCheckBox.IsChecked;
            if (isChecked.HasValue)
            {
                if ((bool)isChecked)
                {
                    SetTestMessage();
                }
            }
        }

        private void TextBox_LostFocus(object sender, RoutedEventArgs e)
        {
            bool? isChecked = MessageCheckBox.IsChecked;
            if (isChecked.HasValue)
            {
                if ((bool)isChecked)
                {
                    SetTestMessage();
                }
            }
        }

        private void Cancel_Clicked(object sender, RoutedEventArgs e)
        {
            var myWindow = Window.GetWindow(this);
            myWindow.Close();
        }
    }
}
