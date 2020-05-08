using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace RX_Common
{
        public static class ScreenExtensions
        {
            public static void GetDpi(this System.Windows.Forms.Screen screen, DpiType dpiType, out uint dpiX, out uint dpiY)
            {
                var pnt = new System.Drawing.Point(screen.Bounds.Left + 1, screen.Bounds.Top + 1);
                var mon = MonitorFromPoint(pnt, 2/*MONITOR_DEFAULTTONEAREST*/);
                GetDpiForMonitor(mon, dpiType, out dpiX, out dpiY);
            }

            //https://msdn.microsoft.com/en-us/library/windows/desktop/dd145062(v=vs.85).aspx
            [DllImport("User32.dll")]
            private static extern IntPtr MonitorFromPoint([In]System.Drawing.Point pt, [In]uint dwFlags);

            //https://msdn.microsoft.com/en-us/library/windows/desktop/dn280510(v=vs.85).aspx
            [DllImport("Shcore.dll")]
            private static extern IntPtr GetDpiForMonitor([In]IntPtr hmonitor, [In]DpiType dpiType, [Out]out uint dpiX, [Out]out uint dpiY);
        }

        //https://msdn.microsoft.com/en-us/library/windows/desktop/dn280511(v=vs.85).aspx
        public enum DpiType
        {
            Effective = 0,
            Angular = 1,
            Raw = 2,
        }

    public class RxScreen
    {
        public static RxScreen Screen = new RxScreen();

        public double Top;
        public double Left;
        public double Width;
        public double Height;
        public double Scale;
        public bool   Surface;
        public bool   Elo;
        public double FactX;
        public double FactY;

        public RxScreen()
        {
            int display;
            display = System.Windows.Forms.Screen.AllScreens.Length-1;
            Left    = System.Windows.Forms.Screen.AllScreens[display].Bounds.Left;
            Width   = System.Windows.Forms.Screen.AllScreens[display].Bounds.Width;
            Top     = System.Windows.Forms.Screen.AllScreens[display].Bounds.Top;
            Height  = System.Windows.Forms.Screen.AllScreens[display].Bounds.Height;

            if (Width!=0) Scale = System.Windows.SystemParameters.PrimaryScreenWidth / Width;
            else          Scale = 1;
            Surface = (Scale==0.5);
            Elo     = (Scale==0.8);
            if (Surface)
            {   
                FactX=0.975;
                FactY=0.90;
            }
            else if (Elo)
            {
                FactX=0.8;
                FactY=0.2;
            }
            else FactX=FactY=Scale=1.0;
        }

        //--- PlaceWindow -----------------------------------------------------------
        public void PlaceWindow(Window wnd)
        {
            if (wnd.WindowStartupLocation==WindowStartupLocation.CenterScreen)
            {
                wnd.Left = Left+(Width-wnd.Width)/2;
                if (wnd.Margin.Bottom>0) wnd.Top = Top+Height-wnd.Height-wnd.Margin.Bottom;
                else                     wnd.Top = (Height-wnd.Height)/2;
                wnd.WindowStartupLocation = WindowStartupLocation.Manual;
                return;
            }
           
            if (wnd.Width>Width) 
            {
                wnd.Width=Width;
                wnd.Left=Left;
            }
            if (wnd.Height>Height)
            {
                wnd.Height=Height;
                wnd.Top=Top;
            }
            if (wnd.Left<Left)
            {
                wnd.Left+=Left;
            }
            if (wnd.Top+wnd.Height>Top+Height)
            {
                wnd.Top = Top+Height-wnd.Height;
            }
        }
    }

}
