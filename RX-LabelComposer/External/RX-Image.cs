using System;
using System.IO;

namespace RX_LabelComposer.External
{
    public class RX_Image
    {
        //--- Property Source ---------------------------------------
        private string _Source;
        public string Source
        {
            get { return _Source; }
            set 
            { 
                if (value!=_Source)
                {
                    _Source = value; 

                    _SrcWidth       = 0;
                    _SrcHeight      = 0;
                    _SrcDpiWidth    = 1;
                    _SrcDpiHeight   = 1;

                    //--- tif file ---
                    try
                    {
                        using(FileStream stream = new FileStream(value, FileMode.Open, FileAccess.Read))
                        {
                            using(System.Drawing.Image tif = System.Drawing.Image.FromStream(stream, false, false))
                            {
                                _SrcWidth  = tif.Width;
                                _SrcHeight = tif.Height;
                                _SrcDpiWidth = (int)tif.HorizontalResolution;
                                _SrcDpiHeight= (int)tif.VerticalResolution;
                                return;
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.Message);
                    }
                }
            }
        }

        //--- Property SrcWidth ---------------------------------------
        private int _SrcWidth;
        public int SrcWidth
        {
            get { return _SrcWidth; }
        }

        //--- Property SrcHeight ---------------------------------------
        private int _SrcHeight ;
        public int SrcHeight 
        {
            get { return _SrcHeight ; }
            set { _SrcHeight = value; }
        }
        

        //--- Property Width ---------------------------------------
        private int _Width;
        public int Width
        {
            get { return _Width; }
            set { _Width = value; }
        }

        //--- Property Height ---------------------------------------
        private int _Height;
        public int Height
        {
            get { return _Height; }
            set { _Height = value; }
        }

        //--- Property SrcDpiWidth ---------------------------------------
        private int _SrcDpiWidth;
        public int SrcDpiWidth
        {
            get { return _SrcDpiWidth; }
            set { _SrcDpiWidth = value; }
        }

        //--- Property SrcDpiHeight ---------------------------------------
        private int _SrcDpiHeight;
        public int SrcDpiHeight
        {
            get { return _SrcDpiHeight; }
            set { _SrcDpiHeight = value; }
        }
        
    }
}
