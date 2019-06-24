using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Services
{
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct SFlzInfo
    {
	    public UInt32 WidthPx;
	    public UInt32 lengthPx;
	    public UInt32 bitsPerPixel;
	    public UInt32 lineLen;		// in bytes
	    public UInt32 aligment;	// 8, 16, 32 ,....
	    public Int64  dataSize;
	    public UInt32 resx;
	    public UInt32 resy;
	    public UInt32 bands;
	    public UInt32 maxBandHeight;
    };
}
