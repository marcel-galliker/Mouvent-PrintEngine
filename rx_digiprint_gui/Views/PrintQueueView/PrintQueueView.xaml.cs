using Infragistics.Controls.Grids;
using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.UserControls;
using System;
using System.Diagnostics;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace RX_DigiPrint.Views.PrintQueueView
{
    /// <summary>
    /// Interaction logic for PrintQueueView.xaml
    /// </summary>
    public partial class PrintQueueView : UserControl
    {
        public  ICommand         CommandDelete       { get; private set; }
        public  ICommand         CommandInsert       { get; private set; }

        private Image _ActImage;
        double        _MaxWidth;
        double        _MaxHeight=250;
        private int   _SelectedItems=0;

        //--- constructor -----------------------------------------------
        public PrintQueueView()
        {
            InitializeComponent();
            DataContext = this;

            PrintQueueGrid.ItemsSource   = RxGlobals.PrintQueue.Queue;
            PrintedQueueGrid.ItemsSource = RxGlobals.PrintQueue.Printed;

            RxGlobals.PrintSystem.PropertyChanged   += PrintSystem_PropertyChanged;
            UpdateGridColumns();

            CommandDelete = new RxCommand(DoDelete);
            CommandInsert = new RxCommand(DoInsert);
            Button_Refresh.Visibility = Visibility.Collapsed;
        }

        //--- PrintSystem_PropertyChanged --------------------------------------------
        void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("PrinterType")) 
                UpdateGridColumns();
        }

        //--- Preview_Loaded ---------------------
        private void Preview_Loaded(object sender, RoutedEventArgs e)
        {
            e.Handled = true;
            Image img = sender as Image;
            if (img!=null)
            {
                double  w=img.ActualWidth;
                double  h=img.ActualHeight;
                Point pt_img  = img.PointToScreen(new Point(0,0));
                Point pt_grid = PrintQueueGrid.PointToScreen(new Point(0,0)); 
                _MaxWidth = pt_grid.X+PrintQueueGrid.ActualWidth-pt_img.X-12;
                double factw = w/_MaxWidth;
                double facth = h/_MaxHeight;

                if (factw<facth) factw=facth;
                if (factw!=0 && factw!=1)
                {
                    img.Width  = w/factw;
                    img.Height = h/factw;
                }
            }
            Main.ManipulationStarting    += Image_ManipulationStarting;
            Main.ManipulationDelta       += Image_ManipulationDelta;
        }
        
        //--- Image_TouchDown ----------------------------------------------
        private void Image_TouchDown(object sender, TouchEventArgs e)
        {
            _ActImage = e.Source as Image;
        }

        //--- Image_ManipulationStarting ---------------------------------------------
        void Image_ManipulationStarting(object sender, ManipulationStartingEventArgs e)
        {
            e.ManipulationContainer = PrintQueueGrid;
            e.Mode = ManipulationModes.Scale | ManipulationModes.Translate;
        }

        //--- Image_ManipulationDelta --------------------------------------------
        private void Image_ManipulationDelta(object sender, ManipulationDeltaEventArgs e)
        {
            var element = e.Source as FrameworkElement;
            if (_ActImage==null) return;
            TransformGroup org=_ActImage.RenderTransform as TransformGroup;
            if (org!=null)
            {
                var val = e.DeltaManipulation;
                ScaleTransform      scale_org       = org.Children[0] as ScaleTransform;
                TranslateTransform  translate_org   = org.Children[1] as TranslateTransform;
                TransformGroup      transform       = new TransformGroup();
                Grid                grid            = _ActImage.Parent as Grid;
                GroupBox            gb              = grid.Parent as GroupBox;
                double x = translate_org.X+val.Translation.X;
                double y = translate_org.Y+val.Translation.Y;
                double sx = scale_org.ScaleX*val.Scale.X;
                double sy = scale_org.ScaleY*val.Scale.Y;
                double w = _ActImage.Width*sx;
                double h = _ActImage.Height*sy;
                double ymax = (h-_ActImage.Height)/2;
                double xmax = (w-_ActImage.Width)/2;
                double minx = _ActImage.Width+gb.Margin.Left+12+(_MaxWidth-w)/2;

                if (y>ymax)  y=ymax;
                if (y<-ymax) y=-ymax;

                if (w>_MaxWidth)
                {
                    grid.Width = _MaxWidth;
                    if (x>xmax) x=xmax;
                    if (x<minx) x= minx; 
                }
                else 
                {
                    grid.Width = w;
                    x=xmax;
                }

                if (sx<1) sx=1;
                if (sy<1) sy=1;

                transform.Children.Add(new ScaleTransform(sx, sy));
                transform.Children.Add(new TranslateTransform(x, y));
                _ActImage.RenderTransform = transform;
            }
        }

        //--- UpdateGridColumns ------------------------------------------------
        private bool? _IsScanning=null;
        public void UpdateGridColumns()
        { 
            if (_IsScanning!=RxGlobals.PrintSystem.IsScanning)
            {
                _IsScanning = RxGlobals.PrintSystem.IsScanning;
                if (RxGlobals.PrintSystem.IsScanning)
                {
                    PrintQueueGrid.Columns["PageStr"        ].Visibility = Visibility.Collapsed;        
                    PrintQueueGrid.Columns["Copies"         ].Visibility = Visibility.Collapsed;        
                    PrintQueueGrid.Columns["ScanLength"     ].Visibility = Visibility.Visible;        

                    PrintedQueueGrid.Columns["PageStr"      ].Visibility = Visibility.Collapsed;  
                    PrintedQueueGrid.Columns["Copies"       ].Visibility = Visibility.Collapsed;        
                    PrintedQueueGrid.Columns["ScanLength"   ].Visibility = Visibility.Visible;        
                    PrintedQueueGrid.Columns["ActPage"      ].Visibility = Visibility.Collapsed;        
                    PrintedQueueGrid.Columns["ActCopy"      ].Visibility = Visibility.Collapsed;        
                    PrintedQueueGrid.Columns["Scans"        ].Visibility = Visibility.Visible;        
                    PrintedQueueGrid.Columns["ScansPrinted" ].Visibility = Visibility.Visible;
                }
                else
                {   
                    PrintQueueGrid.Columns["FileName"       ].Visibility = Visibility.Visible;        
                    PrintQueueGrid.Columns["PageStr"        ].Visibility = Visibility.Collapsed;      
                    PrintQueueGrid.Columns["Copies"         ].Visibility = Visibility.Collapsed;      
                    PrintQueueGrid.Columns["ScanLength"     ].Visibility = Visibility.Visible;        
                    PrintQueueGrid.Columns["Ripped"         ].Visibility = Visibility.Collapsed;      
                    PrintQueueGrid.Columns["Progress"       ].Visibility = Visibility.Visible;        

                    PrintedQueueGrid.Columns["PageStr"      ].Visibility = Visibility.Visible;      
                    PrintedQueueGrid.Columns["Copies"       ].Visibility = Visibility.Collapsed;      
                    PrintedQueueGrid.Columns["ScanLength"   ].Visibility = Visibility.Visible;        
                    PrintedQueueGrid.Columns["State"        ].Visibility = Visibility.Visible;        
                    PrintedQueueGrid.Columns["ActPage"      ].Visibility = Visibility.Collapsed;      
                    PrintedQueueGrid.Columns["ActCopy"      ].Visibility = Visibility.Collapsed;      
                    PrintedQueueGrid.Columns["Scans"        ].Visibility = Visibility.Collapsed;      
                    PrintedQueueGrid.Columns["ScansPrinted" ].Visibility = Visibility.Collapsed;      
                    PrintedQueueGrid.Columns["ProgressStr"  ].Visibility = Visibility.Visible;     
                }
            }            

            if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_LB702_UV)
            {
                ExpansionIndiator.Visibility = ExpansionIndiator2.Visibility = Visibility.Collapsed;
                PrintSettings.Visibility = SettingsTitle.Visibility = Visibility.Visible;
            }
            else 
            {
                ExpansionIndiator.Visibility = ExpansionIndiator2.Visibility = Visibility.Visible;
                PrintSettings.Visibility = SettingsTitle.Visibility = Visibility.Collapsed;
            }
        }

        //--- AllButtons -------------------------------------------------------
        private void AllButtons(Visibility visible)
        {
            Button_Delete.Visibility = visible;
            Button_Up.Visibility     = visible;
            Button_Down.Visibility   = visible;
        }        

        //--- Add_Clicked -------------------------------------------------
        private void Add_Clicked1(object sender, RoutedEventArgs e)
        {
            PrintQueueAdd wnd = new PrintQueueAdd();
            try 
            { 
                if (wnd.ShowDialog()==true)
                    wnd.Item.SendMsg(TcpIp.CMD_ADD_PRINT_QUEUE);
            }
            catch(Exception ex)
            {
                Console.WriteLine("Exception {0}", ex.StackTrace); 
            }
        }
       
        //--- Add_Clicked -------------------------------------------------
        private void Add_Clicked(object sender, RoutedEventArgs e)
        {
            FileOpen.Visibility = (FileOpen.Visibility==Visibility.Visible)? Visibility.Collapsed :  Visibility.Visible;
        }
        
        //--- Refresh_Clicked -----------------------------------
        private void Refresh_Clicked(object sender, RoutedEventArgs e)
        {                        
            FileOpen.Refresh();
        }

        //--- FileOpen_IsVisibleChanged ---------------------------------
        private void FileOpen_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue)
            {
                AllButtons(Visibility.Collapsed);
                Button_Add.IsChecked = true;        
                Button_Refresh.Visibility = Visibility.Visible;
            }   
            else
            {
                Button_Add.IsChecked = false;
                Button_Refresh.Visibility = Visibility.Collapsed;
                foreach(Row row in PrintQueueGrid.Rows) 
                {
                    if (row.IsSelected)
                    {
                        AllButtons(Visibility.Visible);
                        break;
                    }
                }
            }               
        }

        //--- Delete_Clicked -------------------------------------------------
        private void Delete_Clicked_LB702(object sender, RoutedEventArgs e)
        {
            if (_SelectedItems==0) return;
            if (RxMessageBox.YesNo("Delete", "Delete the Items",  MessageBoxImage.Question, false))
            {
                foreach(Row row in PrintQueueGrid.Rows)
                {
                    PrintQueueItem item = row.Data as PrintQueueItem;
                    if (item!=null && item.IsSelected)
                    {
                        item.IsSelected = false;
                        item.SendMsg(TcpIp.CMD_DEL_PRINT_QUEUE);
                    }
                }
                AllButtons(Visibility.Collapsed);
                _update_selected_items();
            }
        }

        //--- Delete_Clicked -------------------------------------------------
        private void Delete_Clicked(object sender, RoutedEventArgs e)
        {
			if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_LB702_UV) Delete_Clicked_LB702(sender, e);
			else if (PrintQueueGrid.ActiveItem!=null)
            {
                if (RxMessageBox.YesNo("Delete", "Delete the Item",  MessageBoxImage.Question, false))
                {
                    if (PrintQueueGrid.ActiveItem!=null)
                        (PrintQueueGrid.ActiveItem as PrintQueueItem).SendMsg(TcpIp.CMD_DEL_PRINT_QUEUE);
                    AllButtons(Visibility.Collapsed);
                }
            }
        }

        //--- Up_Clicked -------------------------------------------------
        private void Up_Clicked(object sender, RoutedEventArgs e)
        {
			if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_LB702_UV)
			{
	            foreach(Row row in PrintQueueGrid.Rows)
	            {
	                PrintQueueItem item = row.Data as PrintQueueItem;
	                if (item!=null && item.IsSelected)
	                    item.SendMsg(TcpIp.CMD_UP_PRINT_QUEUE);
	            }
			}
			else if (PrintQueueGrid.ActiveItem!=null) (PrintQueueGrid.ActiveItem as PrintQueueItem).SendMsg(TcpIp.CMD_UP_PRINT_QUEUE);

            if (PrintedQueueGrid.ActiveItem!=null) 
            {
                PrintQueueItem item = PrintedQueueGrid.ActiveItem as PrintQueueItem;
                item.StartFrom=0;
                item.LengthUnit = item.LengthUnit;
                if (item!=null) item.SendMsg(TcpIp.CMD_ADD_PRINT_QUEUE);
            }
        }

        //--- Down_Clicked -------------------------------------------------
        private void Down_Clicked(object sender, RoutedEventArgs e)
        {
			if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_LB702_UV)
			{
	            int row;
	            for(row = PrintQueueGrid.Rows.Count; row>0; )
	            {
	                row--;
	                if (row>=0)
	                {
	                    PrintQueueItem item = PrintQueueGrid.Rows[row].Data as PrintQueueItem;
	                    if (item!=null && item.IsSelected)
	                        item.SendMsg(TcpIp.CMD_DN_PRINT_QUEUE);
	                }
	            }
			}
			else if (PrintQueueGrid.ActiveItem!=null) (PrintQueueGrid.ActiveItem as PrintQueueItem).SendMsg(TcpIp.CMD_DN_PRINT_QUEUE);
        }

        //--- PrintQueueGrid_SelectedRowsCollectionChanged ---------------------------------------------------
        private void PrintQueueGrid_SelectedRowsCollectionChanged(object sender, SelectionCollectionChangedEventArgs<SelectedRowsCollection> e)
        {
            AllButtons(Visibility.Visible);
			if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_LB702_UV)
			{
            	if (e.NewSelectedItems.Count>0) PrintSettings.DataContext = e.NewSelectedItems[0].Data;
			}
			
            if (e.NewSelectedItems.Count>0)
            {
                foreach(Row row in PrintedQueueGrid.Rows) row.IsSelected = false;
                PrintQueueGrid.ActiveItem = e.NewSelectedItems[0].Data;
			}
        }


        //--- Grid_MouseDown ------------------------------------------------
        private void Grid_MouseDown(object sender, MouseButtonEventArgs e)
        {
            PrintQueueItem selected = ((sender as Grid).DataContext) as PrintQueueItem;
            foreach(Row row in PrintQueueGrid.Rows)
            {
                PrintQueueItem item = row.Data as PrintQueueItem;
                if (item.Equals(selected))
                {
                    PrintSettings.DataContext = item;
                    item.IsSelected = !item.IsSelected;
                }
            }
            _update_selected_items();
            e.Handled = true;
        }

        //--- _update_selected_items --------------------------------------------
        private void _update_selected_items()
        {
            _SelectedItems = 0;
            foreach(Row row in PrintQueueGrid.Rows)
            {
                PrintQueueItem item=row.Data as PrintQueueItem;   
                if (item!=null && item.IsSelected && row.Control!=null) 
                {
                    _SelectedItems++;
                    row.Control.Background =  Brushes.Transparent; // Application.Current.Resources["XamGrid_Selected"] as Brush;
                }
                else
                    row.Control.Background = Brushes.Transparent;
            }
        }

        //--- PrintedQueueGrid_SelectedRowsCollectionChanged -------------------------------------------
        private void PrintedQueueGrid_SelectedRowsCollectionChanged(object sender, SelectionCollectionChangedEventArgs<SelectedRowsCollection> e)
        {
            if (e.NewSelectedItems.Count>0)
            {
                foreach(Row row in PrintQueueGrid.Rows) row.IsSelected = false;

                Button_Delete.Visibility = Visibility.Collapsed;
                Button_Down.Visibility   = Visibility.Collapsed;
//              Button_Up.Visibility   = (PrintedQueueGrid.ActiveItem as PrintQueueItem).Progress>=100 ? Visibility.Visible:Visibility.Collapsed;
                Button_Up.Visibility     = Visibility.Visible;
            }
        }
    
        //--- DoDelete --------------------------------------------
        private void DoDelete(object obj)
        {
            Delete_Clicked(null, null);
        }

        //--- DoInsert --------------------------------------------
        private void DoInsert(object obj)
        {
            Add_Clicked(null, null);
        }

        //--- StackPanel_Loaded ---------------------------------------------
        private void StackPanel_Loaded(object sender, RoutedEventArgs e)
        {
            StackPanel panel = sender as StackPanel;
            if (panel!=null)
            {
                if (RxGlobals.PrintSystem.IsScanning) panel.Children.Insert(0, new FileSettingsScan(){ShowSaveButton=true});
                else                                  panel.Children.Insert(0, new FileSettingsWeb());
            }
        }

        //--- PrintQueueGrid_SizeChanged ----------------------------------------
        private void PrintQueueGrid_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            int limit=400;
            if (e.PreviousSize.Height>limit && e.NewSize.Height<=limit)
            {
                XamGrid grid = sender as XamGrid;
                if (grid!=null)
                {
                    foreach (var item in grid.Rows)
                    {
                        Row row = item as Row;
                        if (row!=null && row.IsExpanded)
                            row.IsExpanded = false;                            
                    }
                }
            }
        }

        //--- PrintQueueGrid_InitializeRow ----------------------------------------------
        private void PrintQueueGrid_InitializeRow(object sender, InitializeRowEventArgs e)
        {
            if (FileOpen.NewFile)
            {
                FileOpen.NewFile = false;
                if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_LB702_UV)
                     e.Row.IsExpanded = false;
                else e.Row.IsExpanded = true;
           //     int row = e.Row.Index;
           //     PrintQueueGrid.Rows[row].IsExpanded = true; 
            }
            PrintQueueItem item = e.Row.Data as PrintQueueItem;
            if (item!=null && RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf) item.PreviewOrientation = 270;
        }

        //--- Rotate_Clicked ------------------------------------
        private void Rotate_Clicked(object sender, RoutedEventArgs e)
        {
            Button button = sender as Button;
            PrintQueueItem item = button.DataContext as PrintQueueItem;
            if (item!=null) item.PreviewOrientation = (item.PreviewOrientation+90)%360;
        }

        private static RxScreen _Screen = new RxScreen();
        private int   _ExpansionRow;
        private bool? _ExpansionValue;
        private void PrintQueueGrid_StylusSystemGesture(object sender, StylusSystemGestureEventArgs e)
        {
            if (e.SystemGesture == SystemGesture.Tap)
            {
                XamGrid grid = sender as XamGrid;
                var pos = e.GetPosition(grid);
                Point pos0 = grid.PointToScreen(new Point(0,0));
                pos.X = pos0.X+pos.X/_Screen.Scale;
                pos.Y = pos0.Y+pos.Y/_Screen.Scale;
                // Debug.WriteLine(string.Format("pos.Y={0} pos0.Y={1}", pos.Y, pos0.Y));
                try 
                {
                    for (int row=0; row<grid.Rows.Count; row++)
                    {
                        Control ctrl = grid.Rows[row].Cells[0].Control;
                        if (ctrl!=null)
                        {
                            Point ctrl_pos=ctrl.PointToScreen(new Point(0,0));
        
                            //  Debug.WriteLine(string.Format("ROW[{0}]: Top={1}, Bottom={2}", row, ctrl_pos.Y, ctrl_pos.Y+2*ctrl.ActualHeight));
                            if (pos.Y >= ctrl_pos.Y && pos.Y <= ctrl_pos.Y+ctrl.ActualHeight/_Screen.Scale)
                            {
                                // Debug.WriteLine("FOUND");
                                grid.Rows[row].IsSelected = true;
                                if (e.StylusDevice.StylusButtons.Count<2) // touch screen
                                {
                                    if (pos.X < ctrl_pos.X)
                                    {
                                        _ExpansionValue = !grid.Rows[row].IsExpanded;
                                        _ExpansionRow = row;
                                  //      Debug.WriteLine(string.Format("{0}:_NewExpansion = {1}", Environment.TickCount, _ExpansionValue));
                                        RxBindable.InvokeDelayed(50, ()=>
                                        {
                                            if (_ExpansionValue!=null) 
                                            {
                                                bool value=(bool)_ExpansionValue;
                                                _ExpansionValue=null;
                                                PrintQueueGrid.Rows[_ExpansionRow].IsExpanded = value;
                                            }

                                        });
                                    }                                
                                }
                                break;
                            }
                        }
                    }   
                }
                catch(Exception) 
                { }
            }

        }

        //--- PrintQueueGrid_RowExpansionChanging ------------------
        private void PrintQueueGrid_RowExpansionChanging(object sender, CancellableRowExpansionChangedEventArgs e)
        {
        //  Debug.WriteLine(string.Format("{0}:Expansion={1}", Environment.TickCount, e.Row.IsExpanded));
            if (_ExpansionValue!=null)
            {
             // Debug.WriteLine(string.Format("{0}:Cancel", Environment.TickCount));
                e.Cancel=true;
            }
        }
    }
}
