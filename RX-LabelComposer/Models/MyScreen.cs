using System;
using System.Runtime.InteropServices;

namespace RX_LabelComposer.Models
{
    class MyScreen
    {
        [DllImport("gdi32.dll", CharSet = CharSet.Auto, SetLastError = true, ExactSpelling = true)]
        public static extern int GetDeviceCaps(IntPtr hDC, int nIndex);

        public enum DeviceCap
        {
            HORZSIZE = 4,
            VERTSIZE = 6,
            HORZRES  = 8,
            VERTRES  = 10,

            /// <summary>
            /// Logical pixels inch in X
            /// </summary>
            LOGPIXELSX = 88,
            /// <summary>
            /// Logical pixels inch in Y
            /// </summary>
            LOGPIXELSY = 90

            // Other constants may be founded on pinvoke.net
        }
    }
}
