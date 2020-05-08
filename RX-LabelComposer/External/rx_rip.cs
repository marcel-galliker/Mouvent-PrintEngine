using RX_LabelComposer.Models.Enums;
using System;
using System.Runtime.InteropServices;
using System.Text;

namespace RX_LabelComposer.External
{
    public struct SBoxHeader
    {
        public BoxTypeEnum boxType;
        public Int32 x;
        public Int32 y;
        public Int32 width;
        public Int32 height;
        public Int32 orientation;
        public Int32 mirror;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string content;
    }

    public struct STextBox
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string font;
        public Int32 size;
        public Int32 lineSpace;
        public AlignmentEnum alignment;	// TA_LEFT | TA_RIGHT | TA_CENTER | TA_NEW_LINE
    }

    public struct SBarcodeBox
    {
	    //--- general -----
        public BcTypeEnum bcType;			// e_BarCType
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
        public Int16[] bar;			// bar width
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
        public Int16[] space;		// space width
        public Int16 stretch;		// module for Matrix codes
        public Int16 filter;
        public Int16 codePage;

	    //--- Paramaters for TEC-It Library ---------------------------------------------------------------------------------
	    //-------------------------	DM			QR			MicroQR			CodeBlock	MicroPDF	PDF			Aztec
        public Int16 check;		//				e_QRECLevel	
        public Int16 size;		//	e_DMSizes	e_QRVersion								e_MPDFVers
        public Int16 format;    //	e_DMFormat	e_QRFormat	e_MQRVersion	e_CBFFormat	e_MPDFMode	e_PDFMode	e_AztecFormat
        public Int16 mask;		//	Encoding Mode	x
        public Int16 rows;		//																	x
        public Int16 cols;		//																	x

        public Int16 ccType;
        public Int16 segPerRow;	// RSS
        public Int16 hex;		

	    //--- human readable ------------------
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string font;
        public Int16 fontSize;
        public Int16 fontAbove;
        public Int16 fontDist;
    } 

    public struct SLayoutBox
    {
        public SBoxHeader   hdr;
        public STextBox     text;
        public SBarcodeBox  bc;

        /// <summary>
        /// Initializes all members with default values.
        /// </summary>
        public void InitWithDefaultValues()
        {
            // Set default values for the BoxHeader
            hdr.width  = 20000;
            hdr.height = 20000;

            // Set default values for the TextBox
            text.alignment = AlignmentEnum.Left;
            text.font      = "Arial";
            text.size      = 12;
            text.lineSpace = 5000;
        }
    }

    public struct SLayoutDef
    {
        public Int32 size;
        public Int32 id;
        public Int32 width;
        public Int32 height;
        public Int32 webWidth;
        public Int32 columns;
        public Int32 columnDist;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string label;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string colorLayer;

        public Int32 boxCnt;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
        public SLayoutBox[] box;
    }

    public struct SRipBmpInfo
    {
	    public Int32 width;
	    public Int32 height;
	    public Int32 bitsPerPixel;
	    public Int32 lineLen;
    };

    class RX_Rip
    {
        public static double DPI  = 1200;

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int rip_init();

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int rip_add_fonts(string path);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr rip_open_xml_local(string path, string tempPath);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int rip_load_layout(IntPtr doc, string tempPath, ref SLayoutDef pLayout);
 
        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int rip_save_layout(IntPtr doc, string tempPath, ref SLayoutDef pLayout);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int rip_copy_files(StringBuilder path, ref SLayoutDef pLayout);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int rip_load_files(IntPtr doc, string dstPath, string filename);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool rip_layout_is_equal(ref SLayoutDef pLayout1, ref SLayoutDef pLayout2);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int rip_set_layout(ref SLayoutDef pLayout, ref SRipBmpInfo pInfo);
        
        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int rip_data(ref SLayoutDef pLayout, Int32 x, Int32 y, IntPtr prxBmp, IntPtr prxBmpLabel, IntPtr prxBmpColor);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int rip_get_bitmap(out int width, out int height, out int bitsperpixel, out IntPtr data, out int dataSize, out int stride);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int  rip_bc_states(BcTypeEnum bc_type);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int rip_get_font_name(int nr, StringBuilder fontname);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool rip_get_usf_size(string path, out UInt32 width, out UInt32 height);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int rip_get_print_env(string path, StringBuilder printEnv);

        [DllImport("rx_rip_lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int rip_bmp_copy(IntPtr srcBmp, int srcWidth, int srcHeight, int srcStride, int srcBppx, int dist, int cnt, IntPtr dstBmp, int dstStride);

    }
}
