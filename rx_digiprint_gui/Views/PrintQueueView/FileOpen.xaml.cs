using Infragistics.Controls.Grids;
using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.UserControls;
using System;
using System.Collections.ObjectModel;
using System.IO;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;
using System.Xml;

namespace RX_DigiPrint.Views.PrintQueueView
{
    /// <summary>
    /// Interaction logic for FileOpen.xaml
    /// </summary>
    public partial class FileOpen : UserControl
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

        private RxDirTree             DirTree;
        private Dispatcher            _dispatcher;
        private System.Timers.Timer   _timer;
        private int                   _PreviewSize=0;
        private int                   _PreviewBusy=0;

        //--- Constructor -----------------------------------------------
        public FileOpen()
        {
            InitializeComponent();

            DataContext = this;

            _dispatcher = Dispatcher;
            _timer = new System.Timers.Timer(10);
            _timer.Elapsed += _timer_Elapsed;

            DirItem.OnPreviewChanged = _preview_changed;
            DirItem.OnPreviewStarted = _preview_started;
            DirItem.OnPreviewDone    = _preview_done;

            RootButton      = Properties.Settings.Default.FileOpen_DataSource;
            _PreviewSize    = Properties.Settings.Default.FileOpen_Size;

            DirGrid.StylusSystemGesture += RxXamGrid.RxStylusSystemGesture;

        //  SmallSize.IsChecked     = (_PreviewSize==0);
            MidSize.IsChecked       = (_PreviewSize==1);
            LargeSize.IsChecked     = (_PreviewSize==2);
            RippedData.IsChecked    = true;
            MsgRefresh.Visibility   = Visibility.Hidden;
            View_Clicked(null, null);
        }

        //--- UserControl_Loaded -------------------------------------------
        private void UserControl_Loaded(object sender, RoutedEventArgs e)
        {
            if (DirTree==null)
            {
                new Thread(() =>
                {
                    RxBindable.Invoke(() =>
                    {
                        DirTree = new RxDirTree();
                        DirTree.ActDirChangedEvent += _ActDirChanged;
                        DirTree.ActDir = Properties.Settings.Default.FileOpen_ActDir;
                        DirTreeGrid.Children.Add(DirTree);
                   //     Refresh();
                    });
                }).Start();
            }
        }

        //--- Property NewFile ---------------------------------------
        private bool _NewFile;
        public bool NewFile
        {
            get { return _NewFile; }
            set { _NewFile=value; }
        }

        //--- _timer_Elapsed ----------------------------------------------------- 
        private void _timer_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            _timer.Enabled = false;
            var cell = DirGrid.ActiveCell;
            _dispatcher.Invoke(()=>DirGrid.InvalidateData());
            if (cell!=null) _dispatcher.Invoke(()=>DirGrid.ScrollCellIntoView(cell));        
        }

        //--- _ActDirChanged --------------------------
        void _ActDirChanged(string dir)
        {
            MsgRefresh.Visibility = Visibility.Visible;
            DirGrid.ItemsSource = null;
            Thread thread=new Thread(()=>
            {
                Thread.Sleep(100);   // give time to display text
                RxBindable.Invoke(()=>
                {
                        dir = Dir.global_path(dir);
                        ActDirCtrl.Text = dir;
                        DirGrid.ItemsSource = _dir = new Dir(dir, (bool)RippedData.IsChecked, dir.Equals(_root)).List;
                        _SetRowHeight(_row_height);
                        MsgRefresh.Visibility = Visibility.Hidden;                
                });
            });
            thread.Start();
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
   
        /*
        //--- Property ActDir -----------------------------------------------
        private string _ActDir;
        public string ActDir
        {
            get { return _ActDir; }
            set 
            {
                if (value==null || value.Equals("")) return;

                _ActDir = value;
                ActDirCtrl.Text = _ActDir;
                DirGrid.ItemsSource = _dir = new Dir(_ActDir, (bool)RippedData.IsChecked, _ActDir.Equals(_root)).List;
                _SetRowHeight(_row_height);
            }
        }
        */

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
            if (_PreviewBusy==1)
                _dispatcher.Invoke(()=>Cursor = System.Windows.Input.Cursors.Wait);
        }   

        //---  _preview_done -----------------------------------------
        private void _preview_done()
        {
            _PreviewBusy--;
            if (_PreviewBusy<=0) 
                _dispatcher.Invoke(()=>Cursor = System.Windows.Input.Cursors.Arrow);
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
                string dir = DirTree.ActDir;
                if (item.FileName.Equals(".."))
                {
                    int i= dir.LastIndexOfAny(new Char[]{'/', '\\'});
                    if (i>0) dir = dir.Remove(i);
                }
                else dir = item.FileName;
                DirTree.ActDir = dir;
            }
            else if (item.FileType==DirItem.ENFileType.RunList)
            {
                _load_runList(item);
            }
            else
            {
                _ActPath = DirTree.ActDir;

                _NewFile = true;
                //--- add file --------------
                {
                    PrintQueueItem pq = new PrintQueueItem();
                
                    pq.FilePath = item.FileName;
                    pq.read_image_properties(item.FileName);
                    pq.LoadDefaults();

                    pq.SendMsg(TcpIp.CMD_ADD_PRINT_QUEUE);
                }

                //--- save actual position ----------------------
                Properties.Settings.Default.FileOpen_DataSource = RootButton;
                Properties.Settings.Default.FileOpen_ActDir     = DirTree.ActDir;
            //    if ((bool)SmallSize.IsChecked)  Properties.Settings.Default.FileOpen_Size=0;
                if ((bool)MidSize.IsChecked)    Properties.Settings.Default.FileOpen_Size=1;
                if ((bool)LargeSize.IsChecked)  Properties.Settings.Default.FileOpen_Size=2;
                Properties.Settings.Default.Save();

                Visibility = Visibility.Hidden;
            }
        }
        //--- _load_runList -----------------------------------------------------
        private void _load_runList(DirItem item)
        {
            XmlTextReader xml;
            string path=Dir.local_path(item.FileName);
            string dir =Path.GetDirectoryName(path) + "\\";
            if (File.Exists(path))
            {
                //--- defaults ---
                xml = new XmlTextReader(path);
                try
                {
                    while(xml.Read())
                    {
                        if (xml.NodeType==XmlNodeType.Element && xml.Name.Equals("RunListsJob"))
                        {
                            for  (int i=0; i<xml.AttributeCount; i++)
                            {
                                xml.MoveToAttribute(i);
                                var prop = GetType().GetProperty(xml.Name);
                            }
                            xml.MoveToElement();
                        }
                        if (xml.NodeType==XmlNodeType.Element && xml.Name.Equals("RunList"))
                        {
                            PrintQueueItem pq = new PrintQueueItem();

                            for  (int i=0; i<xml.AttributeCount; i++)
                            {
                                xml.MoveToAttribute(i);
                                if (xml.Name.Equals("Filename"))
                                {
                                    pq.FilePath = dir +xml.Value;
                                    pq.read_image_properties(pq.FilePath);
                                    pq.LoadDefaults();
                                }
                                if (xml.Name.Equals("CopiesCount"))
                                {
                                    pq.LengthUnit = EPQLengthUnit.copies;
                                    pq.Copies     = Rx.StrToInt32(xml.Value); 
                                }
                                if (xml.Name.Equals("PageStart")) pq.FirstPage = Rx.StrToInt32(xml.Value); 
                                if (xml.Name.Equals("PageEnd"))   pq.LastPage  = Rx.StrToInt32(xml.Value); 
                            }
                            xml.MoveToElement();
                            pq.SendMsg(TcpIp.CMD_ADD_PRINT_QUEUE);
                        }
                    }            
                }

                catch(Exception)
                {
                }            
            }
        }

        //--- _item_delete ------------------------------------------------------
        private void _item_delete(DirItem item)
        {
            string[] name=item.FileName.Split('\\');
            if (name.Length>1 && RxMessageBox.YesNo("Delete", String.Format("Delete {0}?", name[name.Length-1]),  MessageBoxImage.Question, false))
            {
                PrintQueueItem pq = new PrintQueueItem();     
                pq.FilePath = item.FileName;
                pq.SendMsg(TcpIp.CMD_DEL_FILE);
                _dir.Remove(item);
            }
        }

        //--- SourceData_Checked -----------------------------------
        private void SourceData_Checked(object sender, RoutedEventArgs e)
        {
            _root  = _source_data;
            _RootButton = 1;
        //    ActDir = _ActPath  = _root;            
        }

        //--- RippedData_Checked -------------------------------------------
        private void RippedData_Checked(object sender, RoutedEventArgs e)
        {
            _root  = _ripped_data;
            _RootButton = 0;
        //    ActDir = _ActPath  = _root;
        }

        //--- DirGrid_StylusSystemGesture ---------------------------------------
        /*
        private void DirGrid_StylusSystemGesture(object sender, StylusSystemGestureEventArgs e)
        {
            if (e.SystemGesture == SystemGesture.Tap)
            {
                XamGrid grid = sender as XamGrid;
                var pos = e.GetPosition(DirGrid);
                Point pos0 = grid.PointToScreen(new Point(0,0));
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
                                break;
                            }
                        }
                    }   
                catch(Exception) 
                { }
            }
        }
         * */

        //--- Rotate_Clicked ------------------------------------
        private void Rotate_Clicked(object sender, RoutedEventArgs e)
        {
            Button button = sender as Button;
            DirItem item = button.DataContext as DirItem;
            if (item!=null) item.PreviewOrientation = (item.PreviewOrientation+90)%360;
        }

        //--- Print_Clicked ------------------------------------
        private void Print_Clicked(object sender, RoutedEventArgs e)
        {
            Button button = sender as Button;
            DirItem item = button.DataContext as DirItem;
            _item_clicked(item);
        }

        //--- Delete_Clicked ------------------------------------
        private void Delete_Clicked(object sender, RoutedEventArgs e)
        {
            Button button = sender as Button;
            DirItem item = button.DataContext as DirItem;
            _item_delete(item);
        }

        //---- DirGrid_SelectedRowsCollectionChanged -----------------------------------
        private void DirGrid_SelectedRowsCollectionChanged(object sender, SelectionCollectionChangedEventArgs<SelectedRowsCollection> e)
        {
            foreach(var row in e.PreviouslySelectedItems)
            {
                DirItem item=row.Data as DirItem;
                Console.WriteLine("Selection Changed: new={0}", row.Index);
                if (item!=null)
                {
                    item.PrintButtonVisibility  = Visibility.Hidden;
                    item.DeleteButtonVisibility = Visibility.Collapsed;
                }
            }
            foreach(var row in e.NewSelectedItems)
            {
                DirItem item=row.Data as DirItem;
                if (item!=null && !item.IsDirectory)
                {
                    item.PrintButtonVisibility  = Visibility.Visible;
                    item.DeleteButtonVisibility = (RxGlobals.User.UserType>=EUserType.usr_supervisor) ? Visibility.Visible : Visibility.Collapsed;
                    DirGrid.ScrollCellIntoView(row.Cells[0]);
                }
            }
        }

        //--- Refresh -----------------------------------
        public void Refresh()
        {
             MsgRefresh.Visibility = Visibility.Visible;
             RxBindable.Invoke(()=>DirGrid.ItemsSource = null);
             Thread thread=new Thread(()=>
             {
                Thread.Sleep(50);   // give time to display text
                RxBindable.Invoke(()=>
                {
                     DirTree.Refresh();
                     _ActDirChanged(DirTree.ActDir);
                     MsgRefresh.Visibility = Visibility.Hidden;                
                });
             });
             thread.Start();
        }
        
        //--- DirGrid_IsVisibleChanged ----------------------------------------------
        private void DirGrid_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue)
            {
                if (DirGrid.ItemsSource==null) Refresh();

                int i;
                int orientation;
                if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf) orientation=270;
                else orientation=0;
                for (i=0; i<DirGrid.Rows.Count; i++)
                {
                    DirItem item = DirGrid.Rows[i].Data as DirItem;
                    if (item!=null) item.PreviewOrientation=orientation;
                }            
            }
        }

        //--- View_Clicked --------------------------------------------------------------------------
        private bool _Directories=false;

        private void View_Clicked(object sender, RoutedEventArgs e)
        {
            if (_Directories)
            {
                MainGrid.ColumnDefinitions[0].MaxWidth = 200;
                MainGrid.ColumnDefinitions[1].MaxWidth = 10000;
            }
            else
            {
                MainGrid.ColumnDefinitions[0].MaxWidth = 10000;
                MainGrid.ColumnDefinitions[1].MaxWidth = 200;
            }
            _Directories = ! _Directories;
            if (DirGrid.ColumnLayouts.Count>0)
            {
                DirGrid.ColumnLayouts[0].Columns["FileType"].ColumnLayout.ColumnWidth = ColumnWidth.SizeToCells;
            }
        }

        public void Print()
        {
            throw new NotImplementedException();
        }
    }
}
