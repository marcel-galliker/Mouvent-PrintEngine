using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Properties;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Resources;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Threading;

namespace RX_DigiPrint.Models
{
    public class DirItem : RxBindable
    {
        public enum ENFileType : int
        {
            Undef,
            Directory,
            SourceFile,
            DataFile
        };

        private int _MaxHeight=500;
        static public Action OnPreviewChanged;
        static public Action OnPreviewStarted;
        static public Action OnPreviewDone;

        //--- creator ------------------------------------------------
        public DirItem()
        {
        }

        //--- Property IsSelected ---------------------------------------
        private bool _IsSelected=false;
        public bool IsSelected
        {
            get { return _IsSelected; }
            set { SetProperty(ref _IsSelected, value); }
        }
        
        //--- Property FileType ---------------------------------------
        private ENFileType _FileType;
        public ENFileType FileType
        {
            get { return _FileType; }
            set { _FileType=value; }
        }       

        //--- Property DataFileName ---------------------------------------
        public string DataFileName
        {
            get { return _FileName; }
            set 
            { 
                _FileName = value;
                if (IsDirectory) Dimension=null;
                else
                {
                    try
                    {
                        PrintQueueItem pq = new PrintQueueItem();
                        pq.read_image_properties(_FileName);
                        Dimension = string.Format(" {0}mm x {1}mm (dots {2})", pq.SrcWidth, pq.SrcHeight, pq.Dots);
                    }
                    catch(Exception)
                    { 
                        Dimension=null;
                    }
                }

                string preview = Dir.local_path(_FileName + "\\" + Path.GetFileName(_FileName) + ".bmp");
                string labeldef = Dir.local_path(_FileName + "\\" + Path.GetFileName(_FileName) + ".rxd");
                if (!File.Exists(preview)) preview = Dir.local_path(_FileName + "\\" + Path.GetFileName(_FileName) + "_preview.bmp"); // wasatch rip
                if (File.Exists(preview))
                {
                    FileInfo info = new FileInfo(preview);
                    Time = info.LastWriteTime.ToString();
                    _FileType = ENFileType.DataFile;
                    // Preview = new System.Windows.Media.Imaging.BitmapImage(new Uri(preview));                              
                    {
                        string thumb_name = info.Directory +"\\"+ Path.GetFileNameWithoutExtension(info.FullName) + ".bmp";
                        thumb_name = Path.GetTempPath() + "rx_thumb_nails\\"+thumb_name.Remove(0, info.Directory.Root.ToString().Length);
                        
                        try
                        {
                            if (!File.Exists(thumb_name) || !File.GetLastWriteTime(info.FullName).Equals(File.GetLastWriteTime(thumb_name)))
                            {
                                Directory.CreateDirectory(Path.GetDirectoryName(thumb_name));
                                File.Copy(preview, thumb_name, true);
                            }
                            Preview = new System.Windows.Media.Imaging.BitmapImage(new Uri(thumb_name));
                        }
                        catch(Exception)
                        {  
                            Preview = new BitmapImage(new Uri("..\\..\\Resources\\Bitmaps\\MessageBox\\No.ico", UriKind.RelativeOrAbsolute));
                        }
                    }
                }
                else if (File.Exists(labeldef))
                {
                    _FileType = ENFileType.DataFile;
                    Preview = new BitmapImage(new Uri("..\\..\\Resources\\Bitmaps\\BarcodeFile.ico", UriKind.RelativeOrAbsolute));
                }
                else 
                {   
                    _FileType = ENFileType.Directory;
                    Dimension = null;
                    IsDirectory=true;
                }
            }
        }

        //--- Property FileName ---------------------------------------
        private string _FileName;
        public string FileName
        {
            get { return _FileName; }
            set 
            { 
                _FileName = value; 
                FileInfo info = new FileInfo(Dir.local_path(_FileName));

                IsDirectory = (info.Attributes & FileAttributes.Directory) == FileAttributes.Directory;
                if (IsDirectory) FileType = ENFileType.Directory;
                else
                {
                    FileType = ENFileType.SourceFile;
                    Time = info.LastWriteTime.ToString();
                    Thread thread = new Thread(()=>_create_preview(info, this));
                    thread.Start();
                }
            }
        }

        //--- Property Dimension ---------------------------------------
        private string _Dimension = null;
        public string Dimension
        {
            get { return _Dimension; }
            set { SetProperty(ref _Dimension, value); }
        }
        
        //--- _create_preview ---------------------------------------------
        private void _create_preview(FileInfo info, DirItem obj)
        {
// 			string thumb_name = info.FullName + ".png"; // git
            string thumb_name = info.Directory +"\\"+ Path.GetFileNameWithoutExtension(info.FullName) + ".png";
            thumb_name = Path.GetTempPath() + "rx_thumb_nails\\"+thumb_name.Remove(0, info.Directory.Root.ToString().Length);
            Console.WriteLine("thumb_name-mag >>{0}<<", thumb_name);

        
            if (OnPreviewStarted!=null) OnPreviewStarted();
            
            Directory.CreateDirectory(Path.GetDirectoryName(thumb_name));

            FileInfo thumb_info = new FileInfo(thumb_name);
            double width, height;
            if (thumb_info.LastWriteTime < info.LastWriteTime)
            {
                string path = Dir.local_path(_FileName);
                if (_FileName.EndsWith(".rxd"))
                {
                    Label label = new Label();
                    label.Load(path);
                    path = label.Preview;
                }

                Console.WriteLine("File >>{0}<< Read", path);
                Image image;
                try
                {
                    image = Image.FromFile(path);
                }
                catch (Exception)
                {
                    BitmapImage img  = new BitmapImage(new Uri("pack://application:,,,/Resources/Bitmaps/file.png", UriKind.RelativeOrAbsolute));
                    Bitmap bmp = RxImaging.BitmapImage2Bitmap(img);
                    image = Image.FromHbitmap(bmp.GetHbitmap());
                }
                width = image.Width;
                height= image.Height;
                Console.WriteLine("File >>{0}<< w={1}, h={2}", _FileName, image.Width, image.Height);
                if (height>_MaxHeight)
                {
                    height=_MaxHeight;
                    width = image.Width * (height/image.Height);
                }
                Console.WriteLine("File >>{0}<< Create Thumbnail", _FileName);
                Image thumbnail = image.GetThumbnailImage((int)width, (int)height, null, IntPtr.Zero);
                Console.WriteLine("File >>{0}<< Save Thumbnail", _FileName);
                try
                {
                    thumbnail.Save(thumb_name);
                }
                catch(Exception)
                { }
                Console.WriteLine("File >>{0}<< done", _FileName);
            }
            try
            {
                RxBindable.Invoke(()=>obj.Preview = new System.Windows.Media.Imaging.BitmapImage(new Uri(thumb_name)));
            }
            catch(Exception){ }
            
            if (OnPreviewDone!=null) OnPreviewDone();
        }

        //--- Property PreviewOrientation ---------------------------------------
        private int _PreviewOrientation=0;
        public int PreviewOrientation
        {
            get { return _PreviewOrientation; }
            set { SetProperty(ref _PreviewOrientation, value);}
        }

        //--- Property Time ---------------------------------------
        private string _Time;
        public string Time
        {
            get { return _Time; }
            set { _Time = value; }
        }

        //--- Property DirVisible ----------------------
        private bool _IsDirectory=false;
        public bool IsDirectory
        {
            get { return _IsDirectory; }
            set { _IsDirectory = value; }
        }

        //--- Property Preview ----------------------
        private ImageSource _Preview;
        public ImageSource Preview
        {
            get { return _Preview; }
            set 
            { 
                if (_Preview!=value)
                {
                    _Preview = value; 
                    if (OnPreviewChanged!=null) 
                        OnPreviewChanged();
                }
            }
        }

        //--- Property PrintButtonVisibility ---------------------------------------
        private Visibility _PrintButtonVisibility = Visibility.Hidden;
        public Visibility PrintButtonVisibility
        {
            get { return _PrintButtonVisibility; }
            set { SetProperty(ref _PrintButtonVisibility, value);}
        }

        //--- Property PrintButtonVisibility ---------------------------------------
        private Visibility _DeleteButtonVisibility = Visibility.Collapsed;
        public Visibility DeleteButtonVisibility
        {
            get { return _DeleteButtonVisibility; }
            set { SetProperty(ref _DeleteButtonVisibility, value); }
        }
           
    }
}
