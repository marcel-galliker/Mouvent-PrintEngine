using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.PrintQueueView;
using RX_DigiPrint.Views.UserControls;
using RX_LabelComposer.External;
using System;
using System.Collections.Specialized;
using System.Drawing;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;

namespace RX_DigiPrint.Views.TestTableView
{
    public partial class TestTableView : UserControl
    {
        //--- plate dimension in mm -----
        private int _width_mm   = 510;
        private int _height_mm  = 510;

        //--- plate image dimension in pixels ------------
        private int _width=1920;
        private int _height=1080;

        //--- plate position in pixels -------------------
        private int _left=850;
        private int _bottom=670;

        private System.Drawing.Point _tl  = new System.Drawing.Point( 867, 405);
        private System.Drawing.Point _tr  = new System.Drawing.Point(1237, 403);
        private System.Drawing.Point _bl  = new System.Drawing.Point( 850, 683);
        private System.Drawing.Point _br  = new System.Drawing.Point(1202, 683);

        public BitmapImage PrevewImage;

        public TestTableView()
        {        
            InitializeComponent();

            RxGlobals.PrintQueue.Queue.CollectionChanged += OnPrintQueueChanged;
            MouventCommandButtons.StartClicked += Start_Clicked;
            
            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
            Button_UV.DataContext         = RxGlobals.StepperStatus;
        }

        //--- OnPrintQueueChanged -----------------------------------------
        void OnPrintQueueChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (RxGlobals.PrintQueue.Queue.Count>0)
            {
                DataContext = TT_PrintSettings.DataContext = RxGlobals.PrintQueue.Queue[0];
                _update_preview();
            }
        }

        //--- PrintSystem_PropertyChanged --------------------------
        void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("PrinterType"))
            {        
                TT_PrintSettings.Visibility = (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_test_table) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        //--- _update_preview ----------------------------
        private bool _loop=false;
        private string _actFile;
        private Bitmap _displayBmp;
        private void _update_preview()
        {   
            if (_loop)
            {
                _loop=false;
                return;
            }

            PrintQueueItem item = DataContext as PrintQueueItem;
            if (item!=null && RxGlobals.PrintQueue.Queue.Count>0)
            {
                double factx=MachineImg.ActualWidth  / _width;
                double facty=MachineImg.ActualHeight / _height;
                //----- rotate ------
                int width =(int)(item.SrcHeight /_width_mm *(_br.X-_bl.X) * factx);
                int height=(int)(item.SrcWidth  /_height_mm*(_br.Y-_tr.Y) * facty);

    //            TT_PrintSettings.Ripped = item.Ripped;

                if (width!=0 && height!=0)
                {
                    int angle=500/height;
                    int dx = (int)((_tl.X-_bl.X)*factx*item.SrcHeight/_height_mm);

                    if (!item.FilePath.Equals(_actFile))
                    {
                        if ((width<1000 && height<1000) && (item.Preview!=null))
                        {
                            _displayBmp = QuadDistort.Distort(RxImaging.BitmapImage2Bitmap(item.Preview as BitmapImage),
                            new System.Drawing.Point(dx+0,          0),    // top-left
                            new System.Drawing.Point(width,         0),    // top-right
                            new System.Drawing.Point(0,         height),   // bottom-left
                            new System.Drawing.Point(width,     height),   // bottom-right
                            3);
                        }
                        else _displayBmp=null;

                        _actFile = RxGlobals.PrintQueue.Queue[0].FilePath; 
                    }

                    _loop=true;
                //    PreviewImg.Width  = width+dx;
                    PreviewImg.Height = height;
                    PreviewImg.Source = RxImaging.Bitmap2BitmapSource(_displayBmp);
                    return;
                }
            }
            PreviewImg.Source=null;
        }
       
        //--- SetSlidePos ------------------------------
        private void SetSlidePos(int pos)
        {
            System.Windows.Point pm = MachineImg.PointToScreen(new System.Windows.Point(0,0));

            double  stretch=MachineImg.ActualWidth/_width;
            double  comp=40*MachineImg.ActualWidth/800; // slide has different stretch factor!
            pos = 100;  // 0: Front, 100: Back
            SlideImg.Width  = (1220 - 0.4*pos) * stretch;
            SlideImg.Margin = new Thickness(pm.X+(235+comp+0.2*pos)*stretch, 0, 0, (MachineGrid.ActualHeight-MachineImg.ActualHeight)/2+(400+1.5*pos)*stretch);
        }

        //--- TextBox_PreviewMouseDown ---------------------------
        private void Image_Clicked(object sender, RoutedEventArgs e)
        {
            e.Handled=true;
            PrintQueueItem item = DataContext as PrintQueueItem;
            if (item==null)
            {
                item = new PrintQueueItem();
                DataContext = TT_PrintSettings.DataContext = item;
            }

            if (item!=null)
            {                
                Cursor = System.Windows.Input.Cursors.Wait;
                RxFileOpen dlg = new RxFileOpen(){RootButton=Properties.Settings.Default.FileOpen_DataSource, InitDir=Properties.Settings.Default.FileOpen_ActDir};
                bool? ret=dlg.ShowDialog();
                if (ret!=null && ret==true)
                {
                    item.FilePath = dlg.Selected;
                    item.read_image_properties(item.FilePath);
                    item.LoadDefaults();
                
                    //--- save actual position ----------------------
                    Properties.Settings.Default.FileOpen_DataSource = dlg.RootButton;
                    Properties.Settings.Default.FileOpen_ActDir     = dlg.ActDir;
                    Properties.Settings.Default.Save();
                    _update_preview();
                    MachineImg.InvalidateVisual();
                    ImagePath.InvalidateVisual();
                }
                Cursor = System.Windows.Input.Cursors.Arrow;
            }
        }

        //--- Save_Clicked --------------------------------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            PrintQueueItem item = (DataContext as PrintQueueItem);
            int cnt=RxGlobals.PrintQueue.Queue.Count;
            if (item==null) return;

            //---  standard values -----
            item.ID         = 1;
            item.LengthUnit = EPQLengthUnit.copies;
            item.ScanLength = 1;
            item.PrintGoMode=TcpIp.EPrintGoMode.PG_MODE_GAP;
            item.PrintGoDist= 0;
            item.PageWidth  = item.SrcWidth;
            item.PageHeight = item.SrcHeight;

            item.SaveDefaults();
            if (RxGlobals.PrintQueue.Queue.Count>0) item.SendMsg(TcpIp.CMD_SET_PRINT_QUEUE);
            else                                    item.SendMsg(TcpIp.CMD_ADD_PRINT_QUEUE);
        }

        //--- Start_Clicked ------------------------------------------------------------------
        private bool Start_Clicked()
        {
            PrintQueueItem item = (DataContext as PrintQueueItem);
            if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_table && item != null)
            {
                if (RxGlobals.StepperStatus[0].CoverOpen)
                {
                    RxMessageBox.YesNo("Cover", "Cover is open.", MessageBoxImage.Error, true);
                    return false;
                }
                if (item.CuringPasses > 0 && !RxGlobals.StepperStatus[0].UV_Ready)
                {
                    if (!RxMessageBox.YesNo("UV Dryer", "UV Dryer is OFF. Start printing", MessageBoxImage.Question, false))
                        return false;
                }
            }
            // sol: is this okay? Otherwise, the settings for the first print queue item are reset in Save_Clicked!!!
            if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide ||
                RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide_only ||
                RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_table)
            {
                Save_Clicked(this, null);
            }

            return true;
        }

        //--- Cancel_Clicked --------------------------------------------------------------------
        private void Cancel_Clicked(object sender, RoutedEventArgs e)
        {
            if (DataContext!=null)
                (DataContext as PrintQueueItem).SendMsg(TcpIp.CMD_GET_PRINT_QUEUE_ITM);
        }

        //--- UV_Clicked ------------------------------------------------------------------------
        private void UV_Clicked(object sender, RoutedEventArgs e)
        {
            if (Button_UV.IsChecked)
            {
                if (MvtMessageBox.YesNo(RX_DigiPrint.Resources.Language.Resources.UvDryer, RX_DigiPrint.Resources.Language.Resources.SwitchOff,  MessageBoxImage.Question, false))
                {
                    RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ENCODER_UV_OFF);
                }
            }
            else RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ENCODER_UV_ON);
        }


        //--- MoveToLoad_Clicked -------------------------------------------------
        private void MoveToLoad_Clicked(object sender, RoutedEventArgs e)
        {
            (sender as CheckBox).IsChecked=false;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TT_MOVE_LOAD);  
        }

        //--- MoveToCap_Clicked -------------------------------------------------
        private void MoveToCap_Clicked(object sender, RoutedEventArgs e)
        {
            (sender as CheckBox).IsChecked=false;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TT_MOVE_CAP);  
        }

        //--- MoveToPurge_Clicked -------------------------------------------------
        private void MoveToPurge_Clicked(object sender, RoutedEventArgs e)
        {
            (sender as CheckBox).IsChecked=false;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TT_MOVE_PURGE);  
        }

        //--- MoveToAdjust_Clicked -------------------------------------------------
        private void MoveToAdjust_Clicked(object sender, RoutedEventArgs e)
        {
            (sender as CheckBox).IsChecked=false;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TT_MOVE_ADJUST);  
        }

        //--- MoveToTable_Clicked -------------------------------------------------
        private void MoveToTable_Clicked(object sender, RoutedEventArgs e)
        {
            (sender as CheckBox).IsChecked=false;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TT_MOVE_TABLE);  
        }

        //--- Vacuum_Clicked -------------------------------------------------
        private void Vacuum_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TT_VACUUM);  
        }

        //--- MachineImg_LayoutUpdated ------------------------------------
        private void MachineImg_LayoutUpdated(object sender, EventArgs e)
        {
            if (MachineImg.ActualWidth==0) return;

            try
            {
                System.Windows.Point pg = MachineGrid.PointToScreen(new System.Windows.Point(0,0));
                System.Windows.Point pm = MachineImg.PointToScreen(new System.Windows.Point(0,0));
                PreviewImg.Margin = new Thickness(pm.X-pg.X+_left*MachineImg.ActualWidth/_width+10, 0, 0, pm.Y-pg.Y+(_height-_bottom)*MachineImg.ActualHeight/_height);
                SetSlidePos(0);
                _update_preview();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        //--- MachineImg_LayoutUpdated ------------------------------------
        private void MachineImg_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (MachineImg.ActualWidth==0) return;

            try
            {
                System.Windows.Point pg = MachineGrid.PointToScreen(new System.Windows.Point(0,0));
                System.Windows.Point pm = MachineImg.PointToScreen(new System.Windows.Point(0,0));

                //--- buttons --------------------------
                double height=MachineImg.ActualHeight;
                double width=MachineImg.ActualWidth;
                double x0 = pm.X-pg.X;
                double y0 = pm.Y-pg.Y;

                Canvas.SetLeft(Button_Load,    x0+width*0.22-Button_Load.ActualWidth);
                Canvas.SetTop (Button_Load,    y0+height*0.40);

                Canvas.SetLeft(Button_Cap,     x0+width*0.21-Button_Cap.ActualWidth);
                Canvas.SetTop (Button_Cap,     y0+height*0.50);

                Canvas.SetLeft(Button_Purge,     x0+width*0.205-Button_Cap.ActualWidth);
                Canvas.SetTop (Button_Purge,     y0+height*0.57);

                Canvas.SetLeft(Button_Adjust,  x0+width*0.20-Button_Adjust.ActualWidth);
                Canvas.SetTop (Button_Adjust,  y0+height*0.64);

                Canvas.SetLeft(Button_Table,   x0+width*0.55-7-Button_Table.ActualWidth);
                Canvas.SetTop (Button_Table,   y0+height*0.67);

                Canvas.SetLeft(Button_Vacuum,  x0+width*0.55+7);
                Canvas.SetTop (Button_Vacuum,  y0+height*0.67);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);            
            }
        }

        
    }
}
