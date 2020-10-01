using BitMiracle.LibTiff.Classic;
using iTextSharp.text.pdf;
using RX_Common;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.ComponentModel;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Media;
using System.Xml;

namespace RX_DigiPrint.Models
{
    public class PrintQueueItem : RxBindable
    {
        #region Properties

        private Label _Label;
        private static string _TempPath=Path.GetTempPath() + "rx_thumb_nails\\";
        private static bool _First=true;
        private bool _updating=false;

        //--- clearFolder ----------------------------------------------
        private void clearFolder(string FolderName)
        {
            DirectoryInfo dir = new DirectoryInfo(FolderName);

            FileInfo[] files=dir.GetFiles();
            foreach(FileInfo file in files)
            {
                try
                {
                    FileAttributes attributes = File.GetAttributes(file.FullName);
                    if ((attributes & FileAttributes.ReadOnly)==FileAttributes.ReadOnly )
                        File.SetAttributes(file.FullName, attributes & ~FileAttributes.ReadOnly);
                    file.Delete();
                }
                catch(Exception ex)
                {
                    Console.WriteLine("Deleting File>>{0}<< Exception: {1}", file.FullName, ex.Message);
                }
            }

            DirectoryInfo[] directories=dir.GetDirectories();
            foreach (DirectoryInfo directory in directories)
            {
                clearFolder(directory.FullName);
                try
                {
                    directory.Delete();
                }
                catch(Exception ex)
                {
                    Console.WriteLine("Deleting Directory >>{0}<< Exception: {1}", directory.FullName, ex.Message);
                }
            }
        }

        //--- constructor ---------------------------------------------
        public PrintQueueItem()
        {
            if (_First)
            {
                _First = false;
                Thread t = new Thread(() =>
                {
                    Directory.CreateDirectory(_TempPath);
                    clearFolder(_TempPath);
                });
                t.Start();
            }
        }

        public bool _hasPageSettings { get; set; }

        //--- Property IsSelected ---------------------------------------
        private bool _IsSelected = false;
        public bool IsSelected
        {
            get { return _IsSelected; }
            set { SetProperty(ref _IsSelected, value); }
        }
        
        //--- Property Changed ---------------------------------------
        private bool _Changed=false;
        public bool Changed
        {
            get { return _Changed; }
            set { if (!_updating) SetProperty(ref _Changed, value); }
        }

        //--- ID -------------------------------------------
        private Int32 _id;
        public Int32 ID
        {
            get { return _id; }
            set { Changed |= SetProperty(ref _id, value); }
        }
        
        //--- Name ------------------------------------
        private string _FilePath;
        public string FilePath
        {
            get { return _FilePath; }
            set 
            { 
                if (SetProperty(ref _FilePath, value))
                {
                    FileName = Dir.filename(value);
                    Changed = true;
                    Ripped = Dir.isRipped(FilePath);
                    if (Ripped) 
                    {
                        string filename=Path.GetFileName(_FilePath);
                        if (StartPage>1) filename+=string.Format("_P{0:000000}", StartPage);
                        string tempPath=_TempPath+filename+".bmp";
                        string path=Dir.local_path(_FilePath + Path.DirectorySeparatorChar + filename + ".bmp");
                        if (!File.Exists(path)) path = Dir.local_path(_FilePath + Path.DirectorySeparatorChar + filename + "_preview.bmp");
                        if (File.Exists(path))
                        {
                            if (!File.Exists(tempPath)) File.Copy(path, tempPath);
                            PreviewPath = tempPath;
                        }
                    }
                    else
                    {
                        FileInfo info = new FileInfo(Dir.local_path(_FilePath));
                        string preview = info.Directory +"\\"+ Path.GetFileNameWithoutExtension(info.FullName) + ".png";
                        PreviewPath = _TempPath+preview.Remove(0, info.Directory.Root.ToString().Length);
                    }
                    RxBindable.Invoke(()=>{Preview = RxGlobals.PreviewCash.GetPreview(PreviewPath) as ImageSource;});
                }
            }
        }

        //--- Property FileName ---------------------------------------
        private string _FileName;
        public string FileName
        {
            get { return _FileName; }
            set { SetProperty(ref _FileName, value); }
        }
        
        //--- load_label --------------------------------
        public void load_label()
        { 
            if (_FilePath==null) return;
            if (_Label==null) _Label = new Label();
            string[] part = _FilePath.Split('\\');
            string path = Dir.local_path(_FilePath) + "\\" + part[part.Length-1] + ".rxd";
            Variable=_Label.Load(path);
            if (_Variable)
            {
//              RxBindable.Invoke(()=>{PreviewImage = new BitmapImage(new Uri(_Label.Preview));});
//              if (_Label.Bitmap!=null) RxBindable.Invoke(()=>{PreviewBitmap = _Label.Bitmap;});
                if (_Label.Bitmap!=null) RxBindable.Invoke(()=>{Preview = _Label.Bitmap as ImageSource;});
                Copies    = 1;
                SrcPages  = _Label.Pages;
                SrcWidth  = _Label.Width;
                SrcHeight = _Label.Height;
            }
        }

        //--- Property Variable ---------------------------------------
        private bool _Variable;
        public bool Variable
        {
            get { return _Variable; }
//            set { SetProperty(ref _Variable, value); }
            set { 
                    if (SetProperty(ref _Variable, value))
                        Console.WriteLine("Variable={0}", value);
                }
        }

		//--- Property SinglePage ---------------------------------------
		private bool _SinglePage;
		public bool SinglePage
		{
			get { return _SinglePage; }
			set { SetProperty(ref _SinglePage,value); }
		}

		//--- SrcPages ----------------------------------------
		private Int32  _SrcPages=1;
        public Int32 SrcPages
        {
            get { return _SrcPages; }
            set 
            { 
                if (SetProperty(ref _SrcPages, value))
                { 
                    FirstPage=1;
                    LastPage=_SrcPages;
                    do_checks();
                }
            }
        }

        //--- Property SrcWidth ---------------------------------------
        private Double _SrcWidth;
        public Double SrcWidth
        {
            get { return _SrcWidth; }
            set { SetProperty(ref _SrcWidth, Math.Round(value,3)); }
        }

        //--- Property SrcHeight ---------------------------------------
        private Double _SrcHeight;
        public Double SrcHeight
        {
            get { return _SrcHeight; }
            set { SetProperty(ref _SrcHeight, Math.Round(value,3)); }
        }

		//--- Property SrcBitsPerPixel ---------------------------------------
		private int _SrcBitsPerPixel;
		public int SrcBitsPerPixel
		{
			get { return _SrcBitsPerPixel; }
			set { SetProperty(ref _SrcBitsPerPixel,value); }
		}

		//--- Property ScreenOnPrinter ---------------------------------------
		private bool _ScreenOnPrinter=false;
		public bool ScreenOnPrinter
		{
			get { return _ScreenOnPrinter; }
			set { SetProperty(ref _ScreenOnPrinter,value); }
		}

		//--- Property DropSizes ---------------------------------------
		private int _DropSizes;
        public int DropSizes
        {
            get { return _DropSizes; }
            set { SetProperty(ref _DropSizes, value); }
        }
        
        //--- Property Dots ---------------------------------------
        private string _Dots;
        public string Dots
        {
            get 
            { 
                if (_Dots==null || _Dots.Equals("")) return "SML"; 
                return _Dots; 
            }
            set { SetProperty(ref _Dots, value); }
        }

        //--- LargestDot ---------------------
        public int LargestDot
        {
            get
            {
                if (_Dots.Contains("L")) return 3;
                if (_Dots.Contains("M")) return 2;
                if (_Dots.Contains("S")) return 1;
                return 3;
            }
        }

        //--- Property TestDotSize ---------------------------------------
        private int _TestDotSize=1;
	    public int TestDotSize
	    {
		    get { return _TestDotSize;}
		    set { SetProperty(ref _TestDotSize, value);}
	    }
	
        //--- FirstPage ------------------------------------
        private Int32 _FirstPage=1;
        public Int32 FirstPage
        {
            get { return _FirstPage; }
            set { if (SetProperty(ref _FirstPage, value)) do_checks();}
        }

        //--- LastPage ---------------------------------
        private Int32 _LastPage;
        public Int32 LastPage
        {
            get { return _LastPage; }
            set 
            { 
                if(SetProperty(ref _LastPage, value))
                {
              //      if (value>1) 
              //          SrcPages = LastPage-FirstPage+1;
                    do_checks(); 
                }
            }
        }

        //--- StartPage ---------------------------------
        private Int32 _StartPage;
        public Int32 StartPage
        {
            get { return _StartPage; }
            set { if(SetProperty(ref _StartPage, value)) do_checks(); }
        }

        //--- StartFrom ---------------------------------
        private double _StartFrom;
        public double StartFrom
        {
            get { return _StartFrom; }
            set 
            {
                if (SetProperty(ref _StartFrom, value))
                {
                    Changed=true;
                    do_checks();
                }
            }
        }

        //--- do_checks -------------------------------
        private void do_checks()
        {
            Changed=true;
            if (SrcPages<2)
            {
                FirstPage = 1;
                LastPage  = 1;
                StartPage  = 1;
                FirstPageRange = "";
                LastPageRange = "";
            }
            else
            {
                if (StartPage<FirstPage) StartPage = FirstPage;
                if (Copies<1) Copies=1;
             //   if (StartCopy==0) StartCopy=1;
                FirstPageRange = string.Format("({0}..{1})", 1, SrcPages);
                LastPageRange  = string.Format("({0}..{1})", FirstPage, SrcPages);
            }
            FirstPageOk = (FirstPage>SrcPages) ? Brushes.Red:null;
            LastPageOk  = (LastPage<FirstPage || LastPage>SrcPages) ? Brushes.Red:null;

            if (_Orientation==0 || _Orientation==180)
            {                
                PageDistOk = (_PrintGoMode==TcpIp.EPrintGoMode.PG_MODE_LENGTH && _PrintGoDist<_PageHeight) ? Brushes.Red:Brushes.White;
            }
            else
            {
                PageDistOk = (_PrintGoMode==TcpIp.EPrintGoMode.PG_MODE_LENGTH && _PrintGoDist<_PageWidth) ? Brushes.Red:Brushes.White;
            }
        }

        //--- Property FirstPageRange ---------------------------------------
        private string _FirstPageRange;
        public string FirstPageRange
        {
            get { return _FirstPageRange; }
            set { Changed |= SetProperty(ref _FirstPageRange, value); }
        }

        //--- Property LastPageRange ---------------------------------------
        private string _LastPageRange;
        public string LastPageRange
        {
            get { return _LastPageRange; }
            set { Changed |= SetProperty(ref _LastPageRange, value); }
        }

        //--- Property FirstPageOk ---------------------------------------
        private Brush _FirstPageOk;
        public Brush FirstPageOk
        {
            get { return _FirstPageOk; }
            set { Changed |= SetProperty(ref _FirstPageOk, value); }
        }
        
        //--- Property LastPageOk ---------------------------------------
        private Brush _LastPageOk;
        public Brush LastPageOk
        {
            get { return _LastPageOk; }
            set { Changed |= SetProperty(ref _LastPageOk, value); }
        }

        //--- Property LastPageOk ---------------------------------------
        private Brush _PageDistOk;
        public Brush PageDistOk
        {
            get { return _PageDistOk; }
            set { Changed |= SetProperty(ref _PageDistOk, value); }
        }

        //--- PageStr ----------------------------------
        public string PageStr
        {
            get 
            { 
                if (_FirstPage==0) return ""; 
                if (_FirstPage==_LastPage) return  string.Format("{0}", _FirstPage);
                return string.Format("{0}..{1}", _FirstPage, _LastPage);
            }
        }

        //--- Copies -----------------------------
        private Int32 _Copies=1;
        public Int32 Copies
        {
            get { return _Copies; }
            set { Changed |= SetProperty(ref _Copies, value); }
        }

        //--- Property Passes ---------------------------------------
        private int _Passes;
        public int Passes
        {
            get { return _Passes; }
            set { Changed |= SetProperty(ref _Passes, value); }
        }
        
        //--- Property CuringPasses ---------------------------------------
        private int _CuringPasses;
        public int CuringPasses
        {
            get { return _CuringPasses; }
            set { Changed |= SetProperty(ref _CuringPasses, value); }
        }

        //--- Property PenetrationPasses ---------------------------------------
        private int _PenetrationPasses;
        public int PenetrationPasses
        {
            get { return _PenetrationPasses; }
            set 
            { 
                if (SetProperty(ref _PenetrationPasses, value) && _PenetrationPasses>1)
                {
                    Passes= 0x10 | _PenetrationPasses;
                }
            }
        }
        
        //--- Property Speed ---------------------------------------
        private int _Speed;
        public int Speed
        {
            get { return _Speed; }
            set 
            {
                if (value==0) Changed |= SetProperty(ref _Speed, EN_SpeedList.DefaultValue);
                else          Changed |= SetProperty(ref _Speed, value);
            }
        }
        
        //--- Property ScanMode ---------------------------------------
        private EScanMode _ScanMode;
        public EScanMode ScanMode
        {
            get { return _ScanMode; }
            set { Changed |= SetProperty(ref _ScanMode, value); }
        }    

        //--- Property Scans ---------------------------------------
        private Int32 _Scans;
        public Int32 Scans
        {
            get { return _Scans; }
            set { SetProperty(ref _Scans, value); }
        }

        //--- Property ScansPrinted ---------------------------------------
        private Int32 _ScansPrinted;
        public Int32 ScansPrinted
        {
            get { return _ScansPrinted; }
            set { SetProperty(ref _ScansPrinted, value); }
        }

        //--- Property CopiesPrinted ---------------------------------------
        private Int32 _CopiesPrinted;
        public Int32 CopiesPrinted
        {
            get { return _CopiesPrinted; }
            set { SetProperty(ref _CopiesPrinted, value); }
        }

        //--- Property TestImage ---------------------------------------
        private ETestImage _TestImage;
        public ETestImage TestImage
        {
            get { return _TestImage; }
            set { SetProperty(ref _TestImage, value); }
        }
        
        //--- CollateList -------------------------------
        public EN_CollateList CollateList
        {
            get { return new EN_CollateList(); }
        }

        //--- Collate ----------------------------
        private int _Collate=1;
        public int Collate
        {
            get { return _Collate; }
            set { Changed |= SetProperty(ref _Collate, value); }
        }

        //--- ActPage -----------------------------
        private Int32 _ActPage;
        public Int32 ActPage  
        {
            get { return _ActPage; }
            set { Changed |= SetProperty(ref _ActPage, value); }
        }

        //--- ActCopy -----------------------------
        private Int32 _ActCopy;
        public Int32 ActCopy
        {
            get { return _ActCopy; }
            set { Changed |= SetProperty(ref _ActCopy, value); }
        }        

        //--- State -------------------------------
        private EPQState _State;
        public EPQState State
        {
            get { return _State; }
            set { Changed |= SetProperty(ref _State, value); }
        }

        //--- Progress ---------------------------
        private int _Progress;
        public int Progress
        {
            get { return _Progress; }
            set { Changed |= SetProperty(ref _Progress,  value); }
        }

//--- Property ProgressStr ---------------------------------------
        private string _ProgressStr;
        public string ProgressStr
        {
            get { return _ProgressStr; }
            set { SetProperty(ref _ProgressStr, value); }
        }
        
        //--- OrientationList -------------------------------
        public EN_OrientationList OrientationList
        {
            get { return new EN_OrientationList(); }
        }

        //--- Orientation --------------------------------
        private int _Orientation;
        public int Orientation
        {
            get { return _Orientation; }
            set { if (SetProperty(ref _Orientation, value)) do_checks();}
        }

        //--- Property PreviewOrientation ---------------------------------------
        private int _PreviewOrientation=0;
        public int PreviewOrientation
        {
            get { return _PreviewOrientation; }
            set { SetProperty(ref _PreviewOrientation, value); }
        }
        
        //--- Property ScanLength ---------------------------------------
        private double _ScanLength;
        public double ScanLength
        {
            get { return _ScanLength; }
            set { Changed |= SetProperty(ref _ScanLength, value); }
        }

        //--- Property LengthUnit ---------------------------------------
        private EPQLengthUnit _LengthUnit=EPQLengthUnit.copies;
        public EPQLengthUnit LengthUnit
        {
            get { return _LengthUnit; }
            set 
            {
                if (SetProperty(ref _LengthUnit, value))
                {
                    OnPropertyChanged("LengthUnitMM");
                    OnPropertyChanged("LengthUnitCopies");
                    Changed = true;
                }
                
                if (_LengthUnit == EPQLengthUnit.mm && StartFrom == 1) StartFrom = 0;
                if (_LengthUnit == EPQLengthUnit.copies && StartFrom == 0) StartFrom = 1;
            }
        }

        //--- Property LengthUnitMM ---------------------------------------
        public bool LengthUnitMM
        {
            get { return _LengthUnit==EPQLengthUnit.mm; }
            set 
            {
                if (value == true)
                {
                    LengthUnit = EPQLengthUnit.mm;
                }
            }
        }
        
        //--- Property LengthUnitCopies ---------------------------------------
        public bool LengthUnitCopies
        {
            get { return _LengthUnit==EPQLengthUnit.copies; }
            set
            {
                if (value == true)
                {
                    LengthUnit = EPQLengthUnit.copies;
                }
            }
        }

        //--- PageWidth -----------------------------
        private Double _PageWidth;
        public Double PageWidth
        {
            get { return _PageWidth; }
            set { if (SetProperty(ref _PageWidth, Math.Round(value, 3))) do_checks(); }
        }
        
        //--- PageHeight -----------------------------
        private Double _PageHeight;
        public Double PageHeight
        {
            get { return _PageHeight; }
            set { if (SetProperty(ref _PageHeight, Math.Round(value, 3))) do_checks(); }
        }

        //--- PageMargin -----------------------------
        private Double _PageMargin;
        public Double PageMargin
        {
            get { return _PageMargin; }
            set { Changed |= SetProperty(ref _PageMargin, Math.Round(value, 3));}
        }

		//--- Property IsRunList ---------------------------------------
		private bool _IsRunList=false;
		public bool IsRunList
		{
			get { return _IsRunList; }
			set { SetProperty(ref _IsRunList,value); }
		}

		//--- Property PrintGoMode ---------------------------------------
		private TcpIp.EPrintGoMode _PrintGoMode;
        public TcpIp.EPrintGoMode PrintGoMode
        {
            get { return _PrintGoMode; }
            set {  Changed |= SetProperty(ref _PrintGoMode, value); }
        }
        
        //--- PrintGoDist -----------------------------
        private Double _PrintGoDist;
        public Double PrintGoDist
        {
            get { return _PrintGoDist; }
            set { if (SetProperty(ref _PrintGoDist, Rx.DoubleRange(value, 0, 10000))) do_checks(); }
        }

        //--- Property PageNumber ---------------------------------------
        private PageNumber _PageNumber;
        public PageNumber PageNumber
        {
            get { return _PageNumber; }
            set { SetProperty(ref _PageNumber, value); }
        }        
        
        //--- Property PrintChecks ---------------------------------------
        private bool _PrintChecks;
        public bool PrintChecks
        {
            get { return _PrintChecks; }
            set { Changed |= SetProperty(ref _PrintChecks, value); }
        }

        //--- Preview -----------------------------------
        private ImageSource _Preview;
        public ImageSource Preview
        {
            private set { SetProperty(ref _Preview, value);}
            get { return _Preview;}
        }

        //--- Property PreviewPath ---------------------------------------
        private string _PreviewPath;
        public string PreviewPath
        {
            get { return _PreviewPath; }
            set { SetProperty(ref _PreviewPath, value); }
        }

        //--- Property Material ---------------------------------------
        private string _Material;
        public string Material
        {
            get { return _Material; }
            set { SetProperty(ref _Material, value); }
        }

        //--- Property TestMessage ---------------------------------------
        private string _TestMessage;
        public string TestMessage
        {
            get { return _TestMessage; }
            set { SetProperty(ref _TestMessage, value); }
        }
        

        //--- Ripped -------------------------------------------------
        private bool _Ripped;
        public bool Ripped
        {
            get { return _Ripped; }
            private set { SetProperty(ref _Ripped, value); }
        }

        //--- Property Wakeup ---------------------------------------
        private int _Wakeup;
        public int Wakeup
        {
            get { return _Wakeup; }
            set { Changed |= SetProperty(ref _Wakeup, value); }
        }
        
        //--- Property RipState ---------------------------------------
        private string _RipState;
        public string RipState
        {
            get { return _RipState; }
            set { SetProperty(ref _RipState, value); }
        }
        
        #endregion

        private Int32 _GetField(Tiff tif, TiffTag tag)
        {
            FieldValue[] value = tif.GetField(tag);
            return value[0].ToInt();
        }

        //--- _read_tiff_properties ----------------------------------------------
        private bool _read_tiff_properties(string filePath)
        {
            //--- tif file ---
            try
            {
                using (Tiff  tif = Tiff.Open(filePath, "r"))
                {

                    int width = _GetField(tif, TiffTag.IMAGEWIDTH);
                    int res   = _GetField(tif, TiffTag.XRESOLUTION);

                    SrcWidth     = 25.4*_GetField(tif, TiffTag.IMAGEWIDTH) /_GetField(tif, TiffTag.XRESOLUTION);
                    SrcHeight    = 25.4*_GetField(tif, TiffTag.IMAGELENGTH)/_GetField(tif, TiffTag.YRESOLUTION);
                    PageWidth    = SrcWidth;
                    PageHeight   = SrcHeight;
                    SrcPages     = tif.NumberOfDirectories();
                    return true;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                SrcPages = 1;
            }
            return false;     
        }

        //--- read_image_properties ------------------------------------
        public void read_image_properties(string parFilePath)
        {
            string filePath = Dir.local_path(parFilePath);

            if (_FilePath==null) _FilePath=filePath;
            LoadDefaults();

            //--- check for run list ----
			{
                string[] fname = Directory.GetFiles(filePath, "*.rlj");
                IsRunList = (fname.Length>0);
			}

            if (_read_flz_properties(filePath)) return;

            //--- tif file ---
            try
            {
                string[] fname = Directory.GetFiles(filePath, "*.tif");
                if (fname.Length>0 && _read_tiff_properties(fname[0])) return;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            };
        //    if (_read_tiff_properties(filePath)) return;

            //--- label ----------------------------
            {
                _FilePath = filePath;
                load_label();
            }
            if (Variable) 
            {
                PageWidth    = SrcWidth;
                PageHeight   = SrcHeight;
                return;
            }

            //---bmp file -----------------------------
            if (_read_bmp_properties(filePath)) return;


            //--- pdf file -------------
            if (File.Exists(filePath+".pdf"))
            {
                try
                {
                    PdfReader pdfReader = new PdfReader(filePath);
                    SrcPages      = pdfReader.NumberOfPages;
                    SrcWidth      = (25.4*Convert.ToUInt32(pdfReader.GetPageSize(1).Width)/72.0  + 0.5);    // 72 DPI!
                    SrcHeight     = (25.4*Convert.ToUInt32(pdfReader.GetPageSize(1).Height)/72.0 + 0.5);    // 72 DPI!
                    PageWidth     = SrcWidth;
                    PageHeight    = SrcHeight;
                    return;
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                    SrcPages = 1;
                }
            }
        }

        //--- _read_bmp_properties ----------------------------------------------
        private bool _read_bmp_properties(string filePath)
        {
            try
            {
                string[] files = Directory.GetFiles(filePath, "*_?.bmp");

                SrcPages = 1;

                if (files.Length==0) return false;

                using(FileStream stream = new FileStream(files[0], FileMode.Open, FileAccess.Read))
                {
                    using(System.Drawing.Image bmp = System.Drawing.Image.FromStream(stream, false, false))
                    {
                        SrcWidth     = 25.4*bmp.Width/Convert.ToInt32(bmp.HorizontalResolution+0.5);
                        SrcHeight    = 25.4*bmp.Height/Convert.ToInt32(bmp.VerticalResolution+0.5);
                        PageWidth    = SrcWidth;
                        PageHeight   = SrcHeight;
                        return true;
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);                
            }
            return false;     
        }

        //--- _read_flz_properties ----------------------------------------------
        private bool _read_flz_properties(string filePath)
        {
            try
            {
                string[] files = Directory.GetFiles(filePath, "*_*.flz");

                SrcPages = 1;

                if (files.Length==0) return false;

                using(FileStream stream = new FileStream(files[0], FileMode.Open, FileAccess.Read))
                {
                    SFlzInfo info = new SFlzInfo();
                    int size = Marshal.SizeOf(info);
                    Byte[] buf = new Byte[size];
                    stream.Read(buf, 0, size);
                    int len=RxStructConvert.ToStruct(out info, buf);

                    if (len==size)
                    {
                        if (filePath.Contains("-J-0001"))
                            Console.WriteLine("TEST");
                        SrcWidth  = info.WidthPx;
                        SrcHeight = info.lengthPx;
                        if (info.resx<100) info.resx=info.resy;
                        if (info.resx!=0) SrcWidth  = info.WidthPx  * 25.4 / info.resx;
                        if (info.resy!=0) SrcHeight = info.lengthPx * 25.4 / info.resy;
                        PageWidth    = SrcWidth;
                        PageHeight   = SrcHeight;
                        SrcBitsPerPixel = (int)info.bitsPerPixel;
                        ScreenOnPrinter = (info.bitsPerPixel==8);
                        return true;
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);                
            }
            return false;     
        }

        //--- LoadDefaults ----------------------------------------
        public void LoadDefaults()
        {
            XmlTextReader xml;
            string path = Dir.local_path(FilePath + string.Format("\\{0}.xml", Path.GetFileName(_FilePath)));

            if (File.Exists(path))
            {
                //--- defaults ---
                xml = new XmlTextReader(path);
                try
                {
                    while (xml.Read())
                    {
                        if (xml.NodeType == XmlNodeType.Element)
                        {
                            if (xml.Name.Equals("Defaults"))
                            {
                                for (int i = 0; i < xml.AttributeCount; i++)
                                {
                                    xml.MoveToAttribute(i);
                                    var prop = GetType().GetProperty(xml.Name);
                                    if (prop != null) prop.SetValue(this, TypeDescriptor.GetConverter(prop.PropertyType).ConvertFromString(xml.Value));
                                }
                            }
                            else if (xml.Name.Equals("PageNumber"))
                            {
                                PageNumber = new PageNumber(xml);
                            }
                            xml.MoveToElement();
                        }
                    }
                }

                catch (Exception ex)
                {
                    Console.WriteLine("Exception >>{0}<<", ex.Message);
                }
            }
        }

        //--- SaveDefaults --------------------------------------------------------
        public void SaveDefaults()
        {
            try
            {
                string str;
                str = Dir.local_path(FilePath+string.Format("\\{0}.xml", Path.GetFileName(_FilePath)));
                XmlTextWriter xml = new XmlTextWriter(str, null);

                xml.WriteStartDocument();
                xml.WriteStartElement("Defaults");

                    RxSettingsBase.SaveProperty(this, xml, "ScanMode");
                    RxSettingsBase.SaveProperty(this, xml, "Passes");
                    RxSettingsBase.SaveProperty(this, xml, "PenetrationPasses");
                    RxSettingsBase.SaveProperty(this, xml, "CuringPasses");
                    RxSettingsBase.SaveProperty(this, xml, "Speed");
                    RxSettingsBase.SaveProperty(this, xml, "ScanLength");
                //  RxSettingsBase.SaveProperty(this, xml, "PageHeight");
                //  RxSettingsBase.SaveProperty(this, xml, "PageWidth");
                    RxSettingsBase.SaveProperty(this, xml, "PageMargin");
                    RxSettingsBase.SaveProperty(this, xml, "PrintGoMode");
                    RxSettingsBase.SaveProperty(this, xml, "PrintGoDist");
                    RxSettingsBase.SaveProperty(this, xml, "DropSizes");
                    RxSettingsBase.SaveProperty(this, xml, "Dots");
                    RxSettingsBase.SaveProperty(this, xml, "LengthUnit");
                    RxSettingsBase.SaveProperty(this, xml, "FirstPage");
                    RxSettingsBase.SaveProperty(this, xml, "LastPage");
                    RxSettingsBase.SaveProperty(this, xml, "SrcPages");
                    RxSettingsBase.SaveProperty(this, xml, "Wakeup");

                //  RxSettingsBase.SaveProperty(this, xml, "PrintChecks");

                //  PageNumber.SaveDefaults(xml);

                xml.WriteEndElement();
                xml.WriteEndDocument(); 
                xml.Close();
            }
            catch(Exception e)
            { 
                Console.WriteLine("Exception {0}", e.Message);
            }
        }

        private int _PageOffset=0;

        //--- creator -------------------------------------------------
        public PrintQueueItem(TcpIp.sPrintQueueItem msg)
        {
            double progress;
            ID          = msg.id.id;
            ActPage     = msg.id.page;
            ActCopy     = msg.id.copy;

            Variable        = msg.variable!=0;

            // PrintEnv        = msg.printEnv;
            RipState        = msg.ripState;
            SrcPages        = msg.srcPages;
            SrcWidth        = msg.srcWidth/1000.0;
            SrcHeight       = msg.srcHeight/1000.0;
            FirstPage       = msg.firstPage;
            LastPage        = msg.lastPage;
            StartPage       = msg.start.page;
            SinglePage      = (msg.singlePage!=0);
            Copies          = msg.copies;
            //DropSizes       = msg.dropSizes;
            Wakeup          = msg.wakeup;
            Dots            = msg.dots;
            ScanMode        = (EScanMode)(msg.scanMode);
            if (msg.virtualPasses!=0) Passes = 0x10 | msg.passes;
            else                      Passes = msg.passes;
            PenetrationPasses = msg.penetrationPasses;
            CuringPasses    = msg.curingPasses;
            Speed           = msg.speed;
            Collate         = msg.collate;
            State           = msg.state;
            Orientation     = msg.orientation;
            PageWidth       = msg.pageWidth/1000.0;
            PageHeight      = msg.pageHeight/1000.0;
            PageMargin      = msg.pageMargin/1000.0;
            PrintGoMode     = msg.printGoMode;
            PrintGoDist     = msg.printGoDist/1000.0;
            Scans           = msg.scans;
            ScansPrinted    = msg.scansPrinted;
            CopiesPrinted   = msg.copiesPrinted;
            TestImage       = (ETestImage)msg.testImage;
            PrintChecks     = (msg.checks!=0);
            PageNumber      = new PageNumber(msg.pageNumber);
            PageNumber.PropertyChanged += PageNumber_PropertyChanged;

            FilePath        = msg.filepath;

            progress=0;
            if (msg.lengthUnit == EPQLengthUnit.copies)
            {
                LengthUnit = EPQLengthUnit.copies;
                ScanLength = msg.copies;
                if (msg.start.copy==0 || CopiesPrinted == msg.copiesTotal)  
                    StartFrom = 1;
                else                                                        
                    StartFrom = msg.start.copy;
            }
            else
            {
                ScanLength = msg.scanLength/1000.0;
                LengthUnit = EPQLengthUnit.mm;
                if (RxGlobals.PrintSystem.IsScanning) StartFrom = msg.start.scan;
                else if (msg.copiesTotal!=0 &&  CopiesPrinted != msg.copiesTotal) 
                    StartFrom = (ScanLength * ((double)(msg.copiesPrinted+1) / (double)msg.copiesTotal));
                else                                  
                    StartFrom = 0;
            };

            if (LastPage > FirstPage)
            {
                if (RxGlobals.PrintSystem.IsScanning)
                {
                    if (msg.scansPrinted==1) _PageOffset = StartPage;
                    int pages    = (LastPage-FirstPage+1);
                    int skip     = (msg.scans-msg.scansStart+1)*(_PageOffset-1);
                    Scans        = msg.scansTotal;

                    if (Scans!=0 && ScansPrinted>msg.scansStart) progress = 100.0 * (ScansPrinted-msg.scansStart) / (Scans-msg.scansStart);
                    Progress = (int)progress;
                    ProgressStr = string.Format("p{0} ({1}%)", ActPage, Progress);
                }
                else
                {
                    int total=(LastPage-FirstPage+1) * Copies;
                    if (total!=0) progress = 100.0 * msg.copiesPrinted / total;
                    Progress = (int)progress;
                    ProgressStr = string.Format("p{0} c{1} ({2}%)", ActPage, ActCopy, Progress);
                }
            }
            else
            {
                if (msg.scans>0 && (msg.testImage==0 || msg.testImage==7))
                {
                    if (msg.scansPrinted>=msg.start.scan) progress = 100.0 * (msg.scansPrinted-msg.scansStart) / (msg.scans-msg.scansStart);
                }
                else if (msg.copiesTotal > 0) progress = 100.0 * msg.copiesPrinted / msg.copiesTotal;
                if (progress<0) progress=0;
                Progress = (int)progress;
                if (LengthUnitMM)
                {
//                  if (Copies!=0) StartCopy = (int)(ScanLength*StartCopy/Copies);
                    CUnit unit = new CUnit("m");
                    if (ScanLength < 10) ProgressStr = string.Format("{0:n3}{2} ({1:n1}%)", ScanLength * progress * unit.Factor / 100.0, progress, unit.Name);
                    else ProgressStr = string.Format("{0:n1}{2} ({1}%)", ScanLength * progress * unit.Factor / 100.0, Progress, unit.Name);
                }
                else ProgressStr = string.Format("{0}cp ({1}%)", msg.copiesPrinted, Progress);
            }
            Changed     = false;
        }

        //--- Update --------------------------------------------
        public void Update(PrintQueueItem item)
        {
            _updating=true;
            ActPage         = item.ActPage;
            ActCopy         = item.ActCopy;
            Scans           = item.Scans;
            ScansPrinted    = item.ScansPrinted;
            CopiesPrinted   = item.CopiesPrinted;
            RipState        = item.RipState;
            Progress        = item.Progress;
            ProgressStr     = item.ProgressStr;
            State           = item.State;
            PageMargin      = item.PageMargin;
            PrintGoDist     = item.PrintGoDist;
            _updating       = false;
            if (RxGlobals.LH702_View!=null) RxGlobals.LH702_View.UpdatePrintQueueItem(this);
        }

        //--- PageNumber_PropertyChanged -----------------------------
        void PageNumber_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            Changed = true;
        }
        
        //--- SendMsg ---------------------------------------
        public void SendMsg(UInt32 msgId)
        {
            TcpIp.sPrintQueueMsg msg = new TcpIp.sPrintQueueMsg();
            msg.item.id.id      = ID;
            msg.item.filepath   = FilePath;
            msg.item.preview    = "";//_Preview.;
 //           msg.item.printEnv   = PrintEnv;
            if (SrcPages>1)  LengthUnit = EPQLengthUnit.copies;
            msg.item.srcPages       = SrcPages;
            msg.item.srcWidth       = (Int32)(1000*SrcWidth);
            msg.item.srcHeight      = (Int32)(1000*SrcHeight);
            msg.item.firstPage      = FirstPage;
            msg.item.lastPage       = LastPage;
            msg.item.start.page     = StartPage;
            msg.item.singlePage     = Convert.ToByte(SinglePage);
            msg.item.lengthUnit     = LengthUnit;
            msg.item.copies         = Copies;
            msg.item.testMessage    = TestMessage;
            if(SrcPages>1)
            {
                if (RxGlobals.PrintSystem.IsScanning)
                {
                    msg.item.start.page  = (int) StartFrom;
                    msg.item.start.copy  = 1;
                }
                else
                {
                    msg.item.start.page  = StartPage;
                    msg.item.start.copy  = (int) StartFrom;
                }
                msg.item.start.scan  = 0;
            }
            else if (msg.item.lengthUnit == EPQLengthUnit.copies)
            {
                if (ScanLength>0) msg.item.copies = (int)ScanLength;
                msg.item.start.copy = (int) StartFrom;
                msg.item.start.scan = 0;
            }
            else
            {
                msg.item.start.scan = 0;
                msg.item.start.copy = 0;
                msg.item.start.page = 0;  
                msg.item.start.scan = (int) StartFrom;
                if (!RxGlobals.PrintSystem.IsScanning && SrcHeight!=0) msg.item.copiesPrinted = (int)(StartFrom*1000 / SrcHeight);
            }
            if (RxGlobals.PrintSystem.IsScanning)
            {
                msg.item.scanMode           = ScanMode;
                msg.item.passes             = (byte)(Passes&0x0f);
                msg.item.virtualPasses      = (byte)(Passes>>4);
                msg.item.penetrationPasses  = (byte)PenetrationPasses;
                msg.item.curingPasses       = (byte)CuringPasses;
            }
            else
            {
                msg.item.scanMode       = EScanMode.scan_std;
                msg.item.passes         = 1;
                msg.item.virtualPasses  = 0;
                msg.item.curingPasses   = 0;
                msg.item.penetrationPasses = 0;
            }
            msg.item.speed          = Speed;
            msg.item.collate        = (Byte)Collate;
            msg.item.lengthUnit     = LengthUnit;
            msg.item.variable       = Convert.ToByte(Variable);
            // msg.item.dropSizes      = Convert.ToByte(DropSizes);
            msg.item.wakeup         = Convert.ToByte(Wakeup);
            msg.item.orientation    = Convert.ToByte(Orientation);
            msg.item.pageWidth      = (Int32)(1000*PageWidth);
            msg.item.pageHeight     = (Int32)(1000*PageHeight);
            msg.item.pageMargin     = (Int32)(1000*PageMargin);
            msg.item.printGoMode    = PrintGoMode;
            msg.item.printGoDist    = (Int32)(1000*PrintGoDist);
            msg.item.scanLength     = (Int32)(1000*ScanLength);
            msg.item.testImage      = (byte)TestImage;
            msg.item.checks         = Convert.ToInt32(PrintChecks);
            msg.item.dots           = Dots;
            if (PageNumber != null) PageNumber.ToMsg(ref msg.item.pageNumber);

            RxGlobals.RxInterface.SendMsg(msgId, ref msg);

            if (msgId==TcpIp.CMD_SET_PRINT_QUEUE) Changed=false;
        }

        //--- SendBtProdState ---------------------------------------
        public void SendBtProdState()
        {
            RxGlobals.BtProdState.State = (RxBtDef.EPQState)State;
//            RxGlobals.BtProdState.FilePath    = PreviewPath;
            RxGlobals.BtProdState.FilePath    = FilePath;
            RxGlobals.BtProdState.Progress    = Progress;
            RxGlobals.BtProdState.ProgressStr = ProgressStr;
            if (LengthUnitMM)
            {
                if (ScanLength < 10) RxGlobals.BtProdState.CopiesStr = string.Format("{0:n3}m", ScanLength);
                else RxGlobals.BtProdState.CopiesStr = string.Format("{0:n1}m", ScanLength);
            }
            else RxGlobals.BtProdState.CopiesStr = string.Format("{0}cp", Copies);

            RxGlobals.BtProdState.send(true);
        }
    }
}
