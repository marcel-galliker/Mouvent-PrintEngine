using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Media.Imaging;

namespace RX_DigiPrint.Helpers
{
    // souurce: http://stackVAL_OVERFLOW.com/questions/6484357/converting-bitmapimage-to-bitmap-and-vice-versa


    class RxImaging
    {
        //--- BitmapImage2Bitmap -------------------------------------------------------
        public static Bitmap BitmapImage2Bitmap(BitmapImage bitmapImage)
        {
            if (bitmapImage==null)  return null;

            using(MemoryStream outStream = new MemoryStream())
            {
                BitmapEncoder enc = new BmpBitmapEncoder();
                enc.Frames.Add(BitmapFrame.Create(bitmapImage.UriSource));
                enc.Save(outStream);
                System.Drawing.Bitmap bitmap = new System.Drawing.Bitmap(outStream);
                return new Bitmap(bitmap);
            }
        }

        [System.Runtime.InteropServices.DllImport("gdi32.dll")]
        public static extern bool DeleteObject(IntPtr hObject);
        public static BitmapSource Bitmap2BitmapSource(Bitmap bitmap)
        {
            if (bitmap==null) return null;
            IntPtr hBitmap = bitmap.GetHbitmap();
            BitmapSource retval;

            try
            {
                retval = Imaging.CreateBitmapSourceFromHBitmap(
                             hBitmap,
                             IntPtr.Zero,
                             Int32Rect.Empty,
                             BitmapSizeOptions.FromEmptyOptions());
            }
            finally
            {
                DeleteObject(hBitmap);
            }

            return retval;
        }

    }
}
