using Infragistics.Controls.Grids;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for RxFileOpen.xaml
    /// </summary>
    public partial class RxFileOpen : Window
    {
        private static string _root;
        private static string _ActPath;
        private static string _ripped_data = TcpIp.RX_RIPPED_DATA_ROOT.Remove(0,1)+":";
        private static string _source_data = TcpIp.RX_SOURCE_DATA_ROOT.Remove(0,1)+":";

        private RowHeight _dir_height  = new RowHeight(44);
        private RowHeight _small_icons = new RowHeight(50);
        private RowHeight _mid_icons   = new RowHeight(100);
        private RowHeight _large_icons = new RowHeight(200);

        private RowHeight _row_height;
        
        private ObservableCollection<DirItem> _dir;

        private Dispatcher            _dispatcher;
        private System.Timers.Timer   _timer;
        private int                   _ActiveRow;
        private int                   _ActiveCol;
        private int                   _PreviewBusy=0;

        //--- Constructor --------------------
        public RxFileOpen()
        {
            Cursor = System.Windows.Input.Cursors.Wait;
            InitializeComponent();
            DataContext = this;
            _dispatcher = Dispatcher;
            _timer = new System.Timers.Timer(10);
            _timer.Elapsed += _timer_Elapsed;

            MidSize.IsChecked   = true;
        }

        //--- Window_Loaded --------------------------------------------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            RxGlobals.Screen.PlaceWindow(this);
            DirTree.ActDirChangedEvent += _ActDirChanged;
        }

        //--- _ActDirChanged --------------------------
        void _ActDirChanged(string dir)
        {
            ActDir = Dir.global_path(dir);
        }

        //--- Property RootButton ---------------------------------------
        private int _RootButton=-1;
        public int RootButton
        {
            get { return _RootButton; }
            set 
            { 
                if (value!=_RootButton)
                {
                    _RootButton = value;
                    switch(_RootButton)
                    {
                        case 0: RippedData.IsChecked = true; break;
                        case 1: SourceData.IsChecked = true; break;
                    }
                }
            }
        }

        //--- Property Selected ---------------------------------------
        private string _Selected;
        public string Selected
        {
            get { return _Selected; }
            set { _Selected = value; }
        }

        //--- Property InitDir ---------------------------------------
        public string InitDir
        {
            set 
            { 
                ActDir=value;  
                DirTree.ActDir = value;
            }
        }
        
        //--- Property ActDir -----------------------------------------------
        private string _ActDir;
        public string ActDir
        {
            get { return _ActDir; }
            set 
            {
                if (value.Equals("")) return;

                DirItem.OnPreviewChanged = _preview_changed;
                DirItem.OnPreviewStarted = _preview_started;
                DirItem.OnPreviewDone    = _preview_done;

                _ActDir = value; 
                this.Title = _ActDir;
                ActDirCtrl.Text = _ActDir;
                DirGrid.ItemsSource = _dir = new Dir(_ActDir, (bool)RippedData.IsChecked, _ActDir.Equals(_root)).List;
                _SetRowHeight(_row_height);
            }
        }
        
        //--- _SetRowHeight --------------------------------------------
        private void _SetRowHeight(RowHeight height)
        {
            int i;
            _row_height = height;
            for (i=0; i<DirGrid.Rows.Count; i++)
            {
                if (_dir[i].IsDirectory) DirGrid.Rows[i].Height = _dir_height;
                else DirGrid.Rows[i].Height = _row_height;
            }
        }
        
        //---  _preview_changed -----------------------------------------
        private void _preview_changed()
        {
            _timer.Enabled = true;
        }   
            
        //---  _preview_started -----------------------------------------
        private void _preview_started()
        {
            _PreviewBusy++;
            Console.WriteLine("start: _PreviewBusy={0}", _PreviewBusy);
            if (_PreviewBusy==1)
                _dispatcher.Invoke(()=>Cursor = System.Windows.Input.Cursors.Wait);
        }   

        //---  _preview_done -----------------------------------------
        private void _preview_done()
        {
            _PreviewBusy--;
            Console.WriteLine("done: _PreviewBusy={0}", _PreviewBusy);
            if (_PreviewBusy<=0) 
                _dispatcher.Invoke(()=>Cursor = System.Windows.Input.Cursors.Arrow);
        }   

        //--- _timer_Elapsed ----------------------------------------------------- 
        private void _timer_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            _timer.Enabled = false;
            var cell = DirGrid.ActiveCell;
            _dispatcher.Invoke(()=>DirGrid.InvalidateData());
            if (cell!=null) _dispatcher.Invoke(()=>DirGrid.ScrollCellIntoView(cell));        
        }

        //--- Large_Checked ------------------------------------------------
        private void Large_Checked(object sender, RoutedEventArgs e)
        {
            _SetRowHeight(_large_icons);        
        }

        //--- Small_Checked ------------------------------------------------
        private void Mid_Checked(object sender, RoutedEventArgs e)
        {
            MidSize.IsChecked = true;
            _SetRowHeight(_mid_icons);        
        }

        //--- Small_Checked ------------------------------------------------
        private void Small_Checked(object sender, RoutedEventArgs e)
        {
            _SetRowHeight(_small_icons);        
        }

        //--- _item_clicked ------------------------------------------------------
        private void _item_clicked(DirItem item)
        {
            if (item==null) return;

            if (item.IsDirectory)
            {
                if (item.FileName.Equals(".."))
                {
                    int i= ActDir.LastIndexOfAny(new Char[]{'/', '\\'});
                    if (i>0) ActDir = ActDir.Remove(i);
                }
                else ActDir = item.FileName;
               // DirTree.ActDir = ActDir;
            }
            else
            {
                Selected = item.FileName;
                _ActPath = ActDir;

                DialogResult = true; 
            }
        }

        //--- DirGrid_CellDoubleClicked -----------------------------------------
        private void DirGrid_CellDoubleClicked(object sender, Infragistics.Controls.Grids.CellClickedEventArgs e)
        {
            _item_clicked(e.Cell.Row.Data as DirItem);
        }

        //--- Open_Clicked ------------------------------------------------
        private void Open_Clicked(object sender, RoutedEventArgs e)
        {
            _item_clicked(DirGrid.ActiveItem as DirItem);
        }

        //--- SourceData_Checked -----------------------------------
        private void SourceData_Checked(object sender, RoutedEventArgs e)
        {
            _root  = _source_data;
            _RootButton = 1;
            ActDir = _ActPath  = _root;            
        }

        //--- RippedData_Checked -------------------------------------------
        private void RippedData_Checked(object sender, RoutedEventArgs e)
        {
            _root  = _ripped_data;
            _RootButton = 0;
            ActDir = _ActPath  = _root;
        }

        //--- DirGrid_TouchEnter -----------------------------------------------
        private void DirGrid_TouchEnter(object sender, TouchEventArgs e)
        {
            UIElement element = e.Device.Target as UIElement;
            Point pos=element.PointToScreen(new Point(0,0));
            Point grid=DirGrid.PointToScreen(new Point(0,0));
            double x=grid.X;
            for (int row=0; row<DirGrid.Rows.Count; row++)
            {
                if (DirGrid.Rows[row].IsMouseOver)
                {
                    for (int col=0; col<DirGrid.Columns.Count; col++)
                    {
                        double w= DirGrid.Rows[row].Cells[col].Control.ActualWidth;
                        if (x<pos.X && pos.X<x+w)
                        {
                            _ActiveRow=row;
                            _ActiveCol=col;
                            return;
                        }                        
                        x += w;
                    }
                }
            }
        }

        //--- DirGrid_TouchLeave ------------------------------------------------
        private void DirGrid_TouchLeave(object sender, TouchEventArgs e)
        {
            _ActiveRow=-1;
            _ActiveCol=-1;
        }

        //--- DirGrid_StylusSystemGesture ---------------------------------------
        private void DirGrid_StylusSystemGesture(object sender, StylusSystemGestureEventArgs e)
        {
//          if (e.SystemGesture == SystemGesture.HoldEnter)
            if (e.SystemGesture == SystemGesture.Tap)
            {
                if (_ActiveRow>=0 && _ActiveCol>=0)
                {
                    _item_clicked(DirGrid.Rows[_ActiveRow].Data as DirItem);
                }
            }
        }

        private void Window_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue==true)
            {
                Cursor = System.Windows.Input.Cursors.Arrow;
            }
        }
    }
}
