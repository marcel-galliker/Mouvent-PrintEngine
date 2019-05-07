using Infragistics.Windows.DataPresenter;
using RX_Common;
using RX_LabelComposer.Common;
using RX_LabelComposer.Converters;
using RX_LabelComposer.External;
using RX_LabelComposer.Models.Enums;
using RX_LabelComposer.Views;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace RX_LabelComposer.Models
{
    public class LayoutDefinition : RxBindable
    {
        public static LayoutDefinition The_LayoutDef= new LayoutDefinition();
        private static SLayoutDef _LayoutDef;


        private TextPropertiesPage      TextPage;
        private BarcodePropertiesPage   BcPage;
        private BitmapPalette           PaletteBW, PaletteBT;
        public Func<int>                GetActiveRecNo;

        private System.Windows.Threading.DispatcherTimer DispatcherTimer;

        public TextBox ContentBox;

        //--- Delegates ------------------------------------------
        public delegate void LabelSizeChangedDelegate();
        public LabelSizeChangedDelegate LabelSizeChanged = null;


        //--- Constructor ----------------------------
        private LayoutDefinition()
        {
             _LayoutDef.size = Marshal.SizeOf(_LayoutDef);
             _LayoutDef.box = new SLayoutBox[64];
             LabelBox.OnLabelBoxClicked += OnLabelBoxClicked;
             LabelWidth  = 250;
             LabelHeight = 100;
             ActiveBoxNo = -1;
             ShowBoxBorders = true;

             PropertyPage = TextPage;

             //--- Screen resolution --------------------
             System.Drawing.Graphics g = System.Drawing.Graphics.FromHwnd(IntPtr.Zero);
             IntPtr desktop = g.GetHdc();

             int width, height;
             int px_w, px_h;
             width  = MyScreen.GetDeviceCaps(desktop, (int)MyScreen.DeviceCap.HORZSIZE);
             height = MyScreen.GetDeviceCaps(desktop, (int)MyScreen.DeviceCap.VERTSIZE);
             px_w   = MyScreen.GetDeviceCaps(desktop, (int)MyScreen.DeviceCap.HORZRES);
             px_h   = MyScreen.GetDeviceCaps(desktop, (int)MyScreen.DeviceCap.VERTRES);

             ScreenDpiX = 25.4*px_w/width;
             ScreenDpiY = 25.4*px_h/height;
             //--------------------------------------------------------------

             _Zoom = RX_Rip.DPI / ScreenDpiX;
             LabelBox._Zoom = _Zoom;

             //--- bitmap ---------------------------------------
             List<Color> colors = new List<Color>();
             colors.Add(Colors.Transparent);
             colors.Add(Colors.Black);
             PaletteBT = new BitmapPalette(colors);
             colors[0] = Colors.White;
             PaletteBW = new BitmapPalette(colors);

             PropertyChanged += OnPropertyChanged;

             //--- timer ------------------------------------
             DispatcherTimer = new System.Windows.Threading.DispatcherTimer();
             DispatcherTimer.Tick += new EventHandler(dispatcherTimer_Tick);
             DispatcherTimer.Interval = new TimeSpan(0, 0, 0, 0, 50);
             DispatcherTimer.Start();
        }

        //--- dispatcherTimer_Tick -------------------------------------------------
        private void dispatcherTimer_Tick(object sender, EventArgs e)
        {
            if (BoxProperties.Delay>0)
            {
                if ((--BoxProperties.Delay) == 0)
                {
                    UpdateLabel();
                }
            }
        }

        //--- ShowBoxTypeProperties ------------------------------------------------------
        void ShowBoxTypeProperties()
        {
            switch (_ActiveBox.Type)
            {
                case BoxTypeEnum.Text:
                    {
                        TextProperties prop = new TextProperties(_ActiveBox.TextBoxStruct);
                        prop.PropertyChanged += OnPropertyChanged;
                        TextPage     = new TextPropertiesPage(_ActiveBox.No-1, prop);
                        BcPage       = null;
                        PropertyPage = TextPage;
                    }
                    break;

                case BoxTypeEnum.Barcode:
                    {
                        BarcodeProperties prop = new BarcodeProperties(_ActiveBox.BarcodeBoxStruct);
                        prop.PropertyChanged += OnPropertyChanged;
                        BcPage = new BarcodePropertiesPage(_ActiveBox.No-1, prop);
                        PropertyPage = BcPage;
                        TextPage     = null;
                    }
                    break;
            }
        }
        
        //--- OnPropertyChanged ---------------------------------------------------------------------
        void OnPropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (!e.PropertyName.Equals("LabelBitmap") && !e.PropertyName.Equals("ColorMask"))
                UpdateLabel();
            if (e.PropertyName.Equals("Type"))
                ShowBoxTypeProperties();
        }        

        //--- Property Changed ---------------------------------------------------------------------
        private bool _Changed = false;
        public bool Changed
        {
            get { return _Changed; }
            private set { SetProperty(ref _Changed, value);}
        }

        //--- Property Label ---------------------------------------
        private RX_Image _Image = new RX_Image();
        public string Label
        {
            get { return _LayoutDef.label; }
            set 
            { 
//                if (!value.Equals("") && System.IO.Path.GetPathRoot(value).Equals("")) _Image.Source = System.IO.Path.GetTempPath()+value;
//                else _Image.Source=value;
                _Image.Source = new LabelPath_Converter().Convert(value, null, null, null) as string; 

                SetProperty(ref _LayoutDef.label, value); 

                LabelUnused = (_Image.Source.Equals(""));
                if (!LabelUnused)   
                {
                    LabelImgWidth  = _Image.SrcWidth *RX_Rip.DPI / _Image.SrcDpiWidth / _Zoom;
                    LabelImgHeight = _Image.SrcHeight*RX_Rip.DPI / _Image.SrcDpiWidth / _Zoom;

                    LabelWidth  = LabelSrcWidth  = _Image.SrcWidth *25.4  / _Image.SrcDpiWidth;
                    LabelHeight = LabelSrcHeight = _Image.SrcHeight *25.4 / _Image.SrcDpiHeight;
                }
            }
        }

        //--- Property LabelVisible ---------------------------------------
        private bool _LabelVisible = false;
        public bool LabelVisible
        {
            get { return _LabelVisible; }
            set 
            { 
                SetProperty(ref _LabelVisible, value);
            }
        }

        //--- Property LabelUnused ---------------------------------------
        private bool _LabelUnused=true;
        public bool LabelUnused
        {
            get { return _LabelUnused; }
            set { SetProperty(ref _LabelUnused, value); }
        }
        
        //--- Property LabelBackWidth ---------------------------------------
        private double _LabelBackWidth;
        public double LabelImgWidth
        {
            get { return _LabelBackWidth; }
            set { SetProperty(ref _LabelBackWidth, value); }
        }

        //--- Property LabelBackHeight ---------------------------------------
        private double _LabelBackHeight;
        public double LabelImgHeight
        {
            get { return _LabelBackHeight; }
            set { SetProperty(ref _LabelBackHeight, value); }
        }
        
        //--- Property LabelWidth --------------------------------------
        public double LabelWidth
        {
            get { return _LayoutDef.width/1000; }
            set 
            { 
                if (value<10) value=10;
                int width = Convert.ToInt32(value*1000);
                if (width != _LayoutDef.width)
                {
                    if (value>ColumnDist) ColumnDist=value;
                    SetProperty(ref _LayoutDef.width, width);
                    if (LabelSizeChanged!=null) LabelSizeChanged();
                }
            }
        }

        //--- Property LabelHeight ------------------------------------
        public double LabelHeight
        {
            get { return _LayoutDef.height/1000; }
            set 
            { 
                if (value<10) value=10;
                if(SetProperty(ref _LayoutDef.height, Convert.ToInt32(value*1000)) && LabelSizeChanged!=null)
                    LabelSizeChanged(); 
            }
        }

        //--- Property WebWidth ---------------------------------------
        public double WebWidth
        {
            get { return _LayoutDef.webWidth/1000.0; }
            set 
            { 
                if (SetProperty(ref _LayoutDef.webWidth, Convert.ToInt32(value*1000)))
                {
                    WebWidthPx = _LayoutDef.webWidth / 25400.0 * RX_Rip.DPI / _Zoom;
                }
            }
        }

        //--- Property WebWidthPx ---------------------------------------
        private double _WebWidthPx;
	    public double WebWidthPx
	    {
		    get { return _WebWidthPx;}
		    set { SetProperty(ref _WebWidthPx, value);}
	    }

        //--- Property Columns ---------------------------------------
        public int Columns
        {
            get { return _LayoutDef.columns; }
            set { SetProperty(ref _LayoutDef.columns, value);}
        }

        //--- Property ColumnDist ---------------------------------------
        public double ColumnDist
        {
            get { return _LayoutDef.columnDist/1000.0; }
            set 
            { 
                if (value<LabelWidth) value=LabelWidth;
                SetProperty(ref _LayoutDef.columnDist, Convert.ToInt32(value*1000)); 
            }
        }
        
        //--- Property LabelSrcWidth ---------------------------------------
        private double _LabelSrcWidth;
        public double LabelSrcWidth
        {
            get { return _LabelSrcWidth; }
            set { SetProperty(ref _LabelSrcWidth, (int)value); }
        }

        //--- Property LabelSrcHeight ---------------------------------------
        private double _LabelSrcHeight;
        public double LabelSrcHeight
        {
            get {  return _LabelSrcHeight;  }
            set { SetProperty(ref _LabelSrcHeight, value); }
        }
        

        //--- Property ColorLayer ---------------------------------------
        private RX_Image _ColorLayer = new RX_Image();
        private Image _ColorLayerImg = new Image();
        public string ColorLayer
        {
            get { return _LayoutDef.colorLayer; }
            set 
            { 
//                if (!value.Equals("") && System.IO.Path.GetPathRoot(value).Equals("")) _ColorLayer.Source = System.IO.Path.GetTempPath()+value;
//                else _ColorLayer.Source=value;
                _ColorLayer.Source = new LabelPath_Converter().Convert(value, null, null, null) as string;
                SetProperty(ref _LayoutDef.colorLayer, value); 
            }
        }

        //--- Property BoxList -------------------------------------------
        private BoxDefCollection _BoxList = new BoxDefCollection();
        public BoxDefCollection BoxList
        {
            get { return _BoxList; }
            set { SetProperty(ref _BoxList, value); }
        }

        /*
        //--- Property BoxListScrollVisible --------------------------------------
        private ScrollBarVisibility _BoxListScrollVisible = ScrollBarVisibility.Visible;
        public ScrollBarVisibility BoxListScrollVisible
        {
            get { return _BoxListScrollVisible; }
            //            set { SetProperty(ref _BoxListScrollVisible, value); }
            set { _BoxListScrollVisible = value; }
        }
        */

        //--- Property ActiveBocNo ------------------------------------------
        private int _ActiveBoxNo;
        public int ActiveBoxNo
        {
            get { return _ActiveBoxNo; }
            set {
                if (_ActiveBoxNo>=0) SaveBoxProperties();
                if (SetProperty(ref _ActiveBoxNo, value))
                {
                    if (value >= 0 && value<_BoxList.Count) ActiveBox = _BoxList[value];
                    else ActiveBox = null;
                }
            }
        }

        //--- Property ActiveBox ---------------------------------------
        private BoxProperties _ActiveBox;
        public BoxProperties ActiveBox
        {
            get { return _ActiveBox; }
            set {
                if (value==null) 
                {
                    TextPage= null;
                    BcPage  = null;
                }
                if (SetProperty(ref _ActiveBox, value))
                {
                    if (_ActiveBox!=null &&_BoxListGrid != null)
                    {
//                        _BoxListGrid.ActiveDataItem = _ActiveBox;
                        for (int i = 0; i < _BoxListGrid.Records.Count; i++)
                        {
                            _BoxListGrid.Records[i].IsSelected = _BoxList[i].LabelBox.IsSelected = (i == _ActiveBoxNo);
                        }
                        if (_ActiveBoxNo < _BoxListGrid.ScrollInfo.VerticalOffset) _BoxListGrid.ScrollInfo.SetVerticalOffset(_ActiveBoxNo);    
                        ShowBoxTypeProperties();
                    }
                }
            }
        }

        //---  SaveBoxProperties ---------------------------------------------------
        public void SaveBoxProperties()
        {
            if (_ActiveBoxNo>=0 && _ActiveBoxNo<_BoxList.Count && _ActiveBox != null)
            {
                if (TextPage != null) _BoxList[TextPage.BoxNo].TextBoxStruct  = TextPage.TextBoxClass.TextBoxStruct;
                if (BcPage != null)   _BoxList[BcPage.BoxNo].BarcodeBoxStruct = BcPage.BarcodeBoxClass.BarcodeBox;
            }
        }

        //--- Prperty PropertyPage --------------------------------------------------------
        private Page _PropertyPage;
        public Page PropertyPage
        {
            get { return _PropertyPage; }
            set { SetProperty(ref _PropertyPage, value); }
        }

        //--- BoxListInsertAtIndex ----------------------------------------------------------------
        /// <summary>
        /// Insert a new box at the given position.
        /// </summary>
        /// <param name="index">The zero-based index at which the new item should be inserted.</param>
        public void BoxListInsertAtIndex(int index)
        {
            var layoutBox        = new SLayoutBox();
            layoutBox.InitWithDefaultValues();
            BoxProperties box    = new BoxProperties(layoutBox);
            box.No               = index;
            box.LabelBox.ShowBox = _ShowBoxBorders;
            box.PropertyChanged += OnPropertyChanged;
            BoxList.Insert(index, box);
            for (int i = index; i < BoxList.Count; i++)
            {
                _BoxList[i].No = i + 1;
            }
            ActiveBoxNo = -1;
            ActiveBoxNo = index;
            UpdateLabelBoxes();

        }

        //--- BoxListRemove ----------------------------------------------------------
        public void BoxListRemove(int index)
        {
            if (index>=0 && index<BoxList.Count)
            {
                _LabelArea.Children.Remove(_BoxList[index].LabelBox);
                _BoxList.RemoveAt(index);
                for (int i = 0; i < _BoxList.Count; i++)
                    _BoxList[i].No = i + 1;
            }             
            if (index>=_BoxList.Count)
            {
                if (_BoxList.Count>0) index = _BoxList.Count-1;
                else                  index = -1;
            }
            if (index==ActiveBoxNo) ActiveBoxNo = -1;
            ActiveBoxNo = index;

            UpdateLabelBoxes();
            UpdateLabel();
        }

        //--- SetProperties -------------------------------------------------------------
        private static bool updating = false;
        private void SetProperties(SLayoutDef layout)
        {
            updating    = true;
            Label       = layout.label;
            ColorLayer  = layout.colorLayer;
            LabelWidth  = layout.width/1000;
            LabelHeight = layout.height/1000;
            WebWidth    = layout.webWidth/1000;
            Columns     = layout.columns;
            ColumnDist  = layout.columnDist/1000.0;

            _LayoutDef.box = layout.box;

            BoxDefCollection list = new BoxDefCollection();
            for (int i = 0; i < layout.boxCnt; i++)
            {
                BoxProperties box = new BoxProperties(_LayoutDef.box[i]);
                box.No = i + 1;
                box.LabelBox.ShowBox = _ShowBoxBorders;
                box.PropertyChanged += OnPropertyChanged;
                list.Add(box);
            }
            BoxList = list;
            ActiveBoxNo = -1;
            updating = false;
            UpdateLabel();
        }

        //--- BoxList_Cs2Cpp -----------------------------------------------------
        private void BoxList_Cs2Cpp(ref SLayoutDef layoutDef)
        {
            if (layoutDef.box == null) 
                layoutDef.box = new SLayoutBox[64];
            layoutDef.boxCnt = BoxList.Count;
            SaveBoxProperties();
            for (int i = 0; i < _BoxList.Count(); i++)
            {
                _BoxList[i].ToUnmanaged(ref layoutDef.box[i]);
            }
        }

        //--- RemoveLabelBoxes --------------------------------------------------------
        private void RemoveLabelBoxes()
        {
            if (_LabelArea!=null)
            {
                int i;
                for (i = 0; i < _LabelArea.Children.Count; )
                {
                    if (_LabelArea.Children[i].GetType().Name.Equals("LabelBox"))
                    {
                        _LabelArea.Children.RemoveAt(i);
                    }
                    else i++;
                }
            }
        }

        //--- UpdateLabelBoxes -------------------------------------------------------------
        private void UpdateLabelBoxes()
        {
            if (_LabelArea != null)
            {
                RemoveLabelBoxes();

                foreach (BoxProperties box in _BoxList)
                {
                    _LabelArea.Children.Add(box.LabelBox);
                }
            }
        }

        //--- Load ---------------------------------------------------------------------
        private SLayoutDef _LayoutOrg = new SLayoutDef();
        public void Load(IntPtr doc)
        {
            _LayoutOrg.size = Marshal.SizeOf(_LayoutOrg);
             ActiveBox=null;

            if (RX_Rip.rip_load_layout(doc, System.IO.Path.GetTempPath(),  ref _LayoutOrg) == 1) 
                throw new Exception("Data Structures do not match!");

            //--- load new fontlist 
            RX_Rip.rip_add_fonts(System.IO.Path.GetTempPath());                
            FontList reset = new FontList(true);

            SetProperties(_LayoutOrg);
           
            UpdateLabelBoxes();
            Changed = false;
        }

        //--- Save -------------------------------------------------------------
        public void Save(StringBuilder path, IntPtr doc)
        {
            BoxList_Cs2Cpp(ref _LayoutDef);
            RX_Rip.rip_copy_files(path, ref _LayoutDef);
            RX_Rip.rip_save_layout(doc, System.IO.Path.GetTempPath(), ref _LayoutDef);
            _LayoutOrg = _LayoutDef;
            Changed = false;
        }

        //--- Property DpiX -----------------------------------------------
        private double _ScreenDpiX;
        public double ScreenDpiX
        {
            get { return _ScreenDpiX; }
            private set { _ScreenDpiX = value; }
        }

        //--- Property DpiY -----------------------------------------------
        private double _ScreenDpiY;
        public double ScreenDpiY
        {
            get { return _ScreenDpiY; }
            private set { _ScreenDpiY = value; }
        }

        //--- Property Zoom -----------------------------------------------
        private double _Zoom;
        public double Zoom
        {
            get { return _Zoom; }
            set 
            {
                if (SetProperty(ref _Zoom, value))
                {                    
                    LabelImgWidth  = _Image.SrcWidth *RX_Rip.DPI / _Image.SrcDpiWidth / _Zoom;
                    LabelImgHeight = _Image.SrcHeight*RX_Rip.DPI / _Image.SrcDpiWidth / _Zoom;
                    WebWidthPx     = _LayoutDef.webWidth / 25400.0 * RX_Rip.DPI / _Zoom;
                    foreach (BoxProperties box in _BoxList)
                    {
                        box.LabelBox.Zoom = _Zoom;
                    }
                    UpdateLabel();
                }
            }
        }

        //--- Property LabelBmpWidth ---------------------------------------
        private int _LabelBmpWidth;
        public int LabelBmpWidth
        {
            get { return _LabelBmpWidth; }
            set { SetProperty(ref _LabelBmpWidth, value); }
        }

        //--- Property LabelBmpHeight ---------------------------------------
        private int _LabelBmpHeight;
        public int LabelBmpHeight
        {
            get { return _LabelBmpHeight; }
            set { SetProperty(ref _LabelBmpHeight, value); }
        }

        //--- Property ColoredLabel ---------------------------------------
        private bool _ColoredLabel;
        public bool ColoredLabel
        {
            get { return _ColoredLabel; }
            set { SetProperty(ref _ColoredLabel, value); }
        }
        
        //--- UpdateLabel -----------------------------------------------------------------
        private bool processing=false;
        private void UpdateLabel()
        {
            int width, height, bitsperpixel, datasize, stride;
            IntPtr data;
            IntPtr dummy = new IntPtr();
            SRipBmpInfo info = new SRipBmpInfo(){bitsPerPixel=1};

            if (updating || processing || BoxProperties.Delay!=0) return;
                
            //--- save actuan settings and init RIP ---
            BoxList_Cs2Cpp(ref _LayoutDef);
            Changed = _LayoutOrg.size>0 && !RX_Rip.rip_layout_is_equal(ref _LayoutDef, ref _LayoutOrg);
            RX_Dat.dat_seek(RecordNo.The_RecordNo.RecNo);
            RX_Counter.ctr_set_counter(RecordNo.The_RecordNo.CounterNo);
            LabelBitmap.Clear();
            BitmapPalette palette = _LabelVisible?PaletteBT:PaletteBW;
            for (int i=0; i<Columns; i++)
            {
                RX_Dat.dat_read_next_record();
                
                if (_ColoredLabel && _Image.SrcDpiWidth!=0 && _Image.SrcDpiHeight!=0)
                {
                    SLayoutDef layoutDef = _LayoutDef;
                    layoutDef.width  = (int)(25400*_Image.SrcWidth/_Image.SrcDpiWidth);
                    layoutDef.height = (int)(25400*_Image.SrcHeight/_Image.SrcDpiHeight);                                   
                    RX_Rip.rip_set_layout(ref layoutDef, ref info);
                }
                else 
                    RX_Rip.rip_set_layout(ref _LayoutDef, ref info);   // check in BarcodeProperties::OnBarDefaults count of bar and space array!
                RX_Rip.rip_data(ref _LayoutDef, 0, 0, dummy, dummy, dummy);

                //-- load the bitmap and transfer it ---
                int ret = RX_Rip.rip_get_bitmap(out width, out height, out bitsperpixel, out data, out datasize, out stride);

                if (ret == 0 && bitsperpixel == 1)
                {
                    try
                    {
                        LabelBmpWidth  = (int)(width  / _Zoom);
                        LabelBmpHeight = (int)(height / _Zoom);

                        LabelBitmap.Add(BitmapSource.Create(width, height, ScreenDpiX * Zoom, ScreenDpiY * Zoom, PixelFormats.Indexed1, palette, data, datasize, stride));
                    }
                    catch 
                    { 
                    };
                }
                                
                if (CounterDefinition.The_CounterDefinition.Increment==(int)IncrementEnum.INC_perLabel) RX_Counter.ctr_inc_counter();
            }

            processing = false;
            OnPropertyChanged("LabelBitmap");
        }

        //--- ActiveRecNoChanged --------------------------------------------------------
        public void ActiveRecNoChanged(int recNo)
        {
            UpdateLabel();
        }

        //--- Property BoxListGrid ------------------------------------------------------
        private XamDataGrid _BoxListGrid;
        public XamDataGrid BoxListGrid
        {
            get { return _BoxListGrid; }
            set { _BoxListGrid = value; }
        }

        //--- Property LabelArea --------------------------------------------------------
        private LabelArea _LabelArea;
        public LabelArea LabelArea 
        {
            get { return _LabelArea; } 
            set
            {
                RemoveLabelBoxes();
                _LabelArea = value;
                UpdateLabelBoxes();
            }
        }

        //--- Property ShowBoxBorders ---------------------------------------------------
        private bool _ShowBoxBorders;
        public bool ShowBoxBorders
        {
            get { return _ShowBoxBorders; }
            set
            {
                if (SetProperty(ref _ShowBoxBorders, value))
                {
                    for (int i = 0; i < _BoxList.Count(); i++)
                        _BoxList[i].LabelBox.ShowBox = _ShowBoxBorders;
                }
            }
        }

        //--- Property LabelBitmap ----------------------------------------
        private List<BitmapSource> _LabelBitmap = new List<BitmapSource>();
        public List<BitmapSource> LabelBitmap
        {
            get { return _LabelBitmap;  }
        }

        //--- OnFieldSelected --------------------------------------------
        public void OnFieldSelected(string name)
        {
            if (ActiveBox != null)
            {
                string str = "{" + name + "}";
                int i = ContentBox.SelectionStart;
                if (i >= 0)
                {
                    if (ActiveBox.Content!=null) ActiveBox.Content = ActiveBox.Content.Insert(i, str);
                    else ActiveBox.Content = str;
                    ContentBox.SelectionStart = i+str.Length;
                    ContentBox.Focus();
                }
                else ActiveBox.Content += str;
            }
        }

        //--- OnLabelBoxClicked --------------------------------------------
        private void OnLabelBoxClicked(object obj)
        {
            for (int i = 0; i < _BoxList.Count; i++)
            {
                if (obj.Equals(_BoxList[i].LabelBox))
                {
                    ActiveBoxNo = i;
                    if (BoxListGrid != null && i < BoxListGrid.Records.Count)
                    {
                        BoxListGrid.Records[i].IsSelected = true;
                    }
                    return;
                }
            }
        }
    }
}
