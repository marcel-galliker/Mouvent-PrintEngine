using RX_Common;
using RX_DigiPrint.Helpers;
using RX_LabelComposer.External;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Xml;


namespace RX_DigiPrint.Models
{
    public class Label : RxBindable
    {
        [DllImport("gdi32")]
        private static extern IntPtr CreatePalette(LOGPALETTE2 lplgpl);

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private class LOGPALETTE2
        {
           public short palVersion;
           public short palNumEntries;
           public ulong palColor0;
           public ulong palColor1;
        }

        static private string           _TempPath;
        static private BitmapPalette    _Palette;
        static IntPtr hPalette          = IntPtr.Zero;
        private SLayoutDef              _Layout = new SLayoutDef();  
        private SFileDef                _FileDef = new SFileDef();

        //--- Constructor ---------------------------
        public Label()
        {
            if (_TempPath==null)
            {
                _TempPath = System.IO.Path.GetTempPath();

                List<System.Windows.Media.Color> colors = new List<System.Windows.Media.Color>();
                colors.Add(Colors.Transparent);
                colors.Add(Colors.Black);
                _Palette = new BitmapPalette(colors);

                LOGPALETTE2 pal = new LOGPALETTE2();
                pal.palVersion = 0x300;
                pal.palNumEntries = 2;
                pal.palColor0 = rgba(Colors.Blue);
                pal.palColor1 = rgba(Colors.Yellow);
                hPalette = CreatePalette(pal);
            }
        }

        //--- rgba --------------------------
        private UInt32 rgba(System.Windows.Media.Color c)
        {
            return (UInt32)(c.R | c.G<<8 | c.B<<16 | 0x00<<24);
        }

        //--- Load ---------------------------------------
        public bool Load(string path)
        {
            IntPtr doc;

            Console.WriteLine("Load ({0}) {1}.{2} start", path, DateTime.Now.Second, DateTime.Now.Millisecond);

            try
            {
                if (RxPing.HostReachable(path))
                {
                    System.Windows.Forms.Cursor.Current = System.Windows.Forms.Cursors.WaitCursor;
            
                    doc = RX_Rip.rip_open_xml_local(path, _TempPath);
                    Console.WriteLine("Load ({0}) {1}.{2} 1", path, DateTime.Now.Second, DateTime.Now.Millisecond);
                    if (doc!=IntPtr.Zero)
                    {
                        _FileDef.size = Marshal.SizeOf(_FileDef);
                        RX_Dat.dat_load_file_def(doc, _TempPath, ref _FileDef);
                        _Layout.size = Marshal.SizeOf(_Layout);
                        RX_Rip.rip_load_layout(doc, _TempPath, ref _Layout);
                        RX_Rip.rip_load_files (doc, _TempPath, _Layout.label);
                    //    LoadBitmap();
                        System.Windows.Forms.Cursor.Current = System.Windows.Forms.Cursors.Default;
                        Console.WriteLine("Load ({0}) {1}.{2} 2", path, DateTime.Now.Second, DateTime.Now.Millisecond);
                        RX_Doc.rx_xml_free(doc);
                        return true;
                    }
                    Console.WriteLine("Load ({0}) {1}.{2} 3", path, DateTime.Now.Second, DateTime.Now.Millisecond);
                    System.Windows.Forms.Cursor.Current = System.Windows.Forms.Cursors.Arrow;
                }
                Console.WriteLine("Load ({0}) {1}.{2} end", path, DateTime.Now.Second, DateTime.Now.Millisecond);
            }
            catch(Exception e)
            {
                Console.WriteLine("Exception {0}", e.Message);
                return _read_xml(path);
            }
            return false;
        }

        //--- _read_attribute ---------------------------------------
        private void _read_attribute(string line, string name, ref int val)
        {
            if (line.Contains(name))
            {
                string[] list=line.Split('"');
                string name1=list[0].TrimStart().TrimEnd('=', ' ');
                if (name1.Equals(name)) 
                    Int32.TryParse(list[1], out val);
            }
        }

        //--- _read_xml --------------------------------
        private bool _read_xml(string path)
        {
            _FileDef.recordCnt  = 0;
            _Layout.width       = 0;
            _Layout.height      = 0;
            try 
            { 
                StreamReader xml = new StreamReader(path);
                bool file=false;
                bool layout=false;
                while(!xml.EndOfStream)
                {
                    string line=xml.ReadLine();

                    if (line.Contains("<Layout")) layout=true;
                    if (line.Contains(">"))       layout=false;

                    if(layout)
                    {
                        _read_attribute(line, "Width", ref _Layout.width);
                        _read_attribute(line, "Height", ref _Layout.height);
                    }

                    if (line.Contains("<File"))   file=true;
                    if (line.Contains("</File>")) file=false;
                    if (file)
                    {
                        _read_attribute(line, "RecordCnt", ref _FileDef.recordCnt);
                    }
                };
                return (_FileDef.recordCnt>0);
            }
            catch(Exception)
            {
            }
            return false;
        }

        //--- Property Width ---------------------------------------
        public double Width
        {
            get { return (double)_Layout.width/1000.0; }
        }

        //--- Property Height ---------------------------------------
        public double Height
        {
            get { return (double)_Layout.height/1000.0; }
        }

        //--- Property Pages ---------------------------------------
        public Int32 Pages
        {
            get 
            { 
                if (_Layout.columns==0) return _FileDef.recordCnt;
                else                    return (_FileDef.recordCnt+_Layout.columns-1)/_Layout.columns; 
            }
        }

        //--- Property Preview ---------------------------------------
        public string Preview
        {
            get { return _TempPath + _Layout.label; }
        }

        //--- Property Bitmap ---------------------------------------
        private BitmapSource _Bitmap;
        public BitmapSource Bitmap
        {
            get { return _Bitmap; }
            set { SetProperty(ref _Bitmap, value); }
        }
        
        //--- LoadBitmap --------------------------------------------
        private void LoadBitmap()
        {        
            int width, dist, stride, bppx;   

            Console.WriteLine("LoadBitmap {0}.{1} start", DateTime.Now.Second, DateTime.Now.Millisecond);

            //  RX_Rip.RIP_SetLayout(ref _Layout);
            //  int ret = RX_Rip.RIP_GetBitmap(out width, out height, out bitsperpixel, out data, out datasize, out stride);
            RxBindable.Invoke(()=>
            {                           
                System.Windows.Media.PixelFormat pxfmt;
                Bitmap bmp = new Bitmap(Preview);
                Rectangle rect = new Rectangle(0, 0, bmp.Width, bmp.Height);
                System.Drawing.Imaging.BitmapData bmpData = bmp.LockBits(rect, System.Drawing.Imaging.ImageLockMode.ReadOnly, bmp.PixelFormat);
                if (_Layout.columns>1)
                {
                    switch (bmp.PixelFormat)
                    {
                        case System.Drawing.Imaging.PixelFormat.Format24bppRgb:     bppx=24; pxfmt=PixelFormats.Bgr24; break;
                        default:                                                    bppx=24; pxfmt=PixelFormats.Bgr24; break;
                    }
                    dist  = (int)(_Layout.columnDist*RX_Rip.DPI/25400);
                    width = bmp.Width + (_Layout.columns-1)*dist;
                    stride = (width*bppx)/8;
                    stride = (((stride+3)/4)*4);
                    IntPtr data = Marshal.AllocHGlobal(bmp.Height*stride);
                    RX_Rip.rip_bmp_copy(bmpData.Scan0, bmp.Width, bmp.Height, bmpData.Stride, 24, dist, _Layout.columns, data, stride);

                    Bitmap = BitmapSource.Create(width, bmp.Height, RX_Rip.DPI, RX_Rip.DPI, pxfmt, null, data, bmp.Height*stride, stride);                    

                    Marshal.FreeHGlobal(data);
                }
                else Bitmap = BitmapSource.Create(bmp.Width, bmp.Height, RX_Rip.DPI, RX_Rip.DPI, PixelFormats.Bgr24, null, bmpData.Scan0, bmpData.Stride*bmp.Height, bmpData.Stride);
                bmp.UnlockBits(bmpData);
            }
            );
            Console.WriteLine("LoadBitmap {0}.{1} end", DateTime.Now.Second, DateTime.Now.Millisecond);
        }
        
    }
}
