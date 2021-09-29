using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Windows.Interop;

namespace RX_DigiPrint.Views.SiemensHMI
{
    public class Win32ControlHost : HwndHost
    {
        #region DllImport
        [DllImport("user32.dll", CharSet = CharSet.Unicode)]
        internal static extern IntPtr CreateWindowEx(int dwExStyle,
                                                      string lpszClassName,
                                                      string lpszWindowName,
                                                      int style,
                                                      int x, int y,
                                                      int width, int height,
                                                      IntPtr hwndParent,
                                                      IntPtr hMenu,
                                                      IntPtr hInst,
                                                      [MarshalAs(UnmanagedType.AsAny)] object pvParam);

        [DllImport("user32.dll", CharSet = CharSet.Unicode)]
        internal static extern bool DestroyWindow(IntPtr hwnd);

        [DllImport("user32.dll")]
        public static extern IntPtr SetParent(IntPtr hWndChild, IntPtr hWndNewParent);

        [DllImport("user32.dll")]
        private static extern int FindWindow(string sClass, string sWindow);

        [DllImport("user32.dll", SetLastError = true)]
        public static extern bool MoveWindow(IntPtr hWnd, int X, int Y, int nWidth, int nHeight, bool bRepaint);

        internal const int
          WS_CHILD = 0x40000000,
          WS_VISIBLE = 0x10000000,
          HOST_ID = 0x00000002;
        #endregion

        private IntPtr hwndControl;
        private IntPtr hwndHost;
        private readonly String windowName;
        private readonly String appPath;

        public Win32ControlHost(String windowName, String appPath = "")
        {
            this.windowName = windowName;
            this.appPath = appPath;
            hwndControl = IntPtr.Zero;
        }

        public void Resize(double nWidth, double nHeight)
        {
            MoveWindow(hwndHost, 0, 0, (int)nWidth, (int)nHeight, true);
            MoveWindow(hwndControl, 0, 0, (int)nWidth, (int)nHeight, true);
        }

        protected override HandleRef BuildWindowCore(HandleRef hwndParent)
        {
            // Create the host window
            hwndHost = CreateWindowEx(0, "static", "",
                                        WS_CHILD | WS_VISIBLE,
                                        0, 0,
                                        100, 100,
                                        hwndParent.Handle,
                                        (IntPtr)HOST_ID,
                                        IntPtr.Zero,
                                        0);

            // Wait for the child window to be present and dock it inside our host
            Task.Run(async () =>
            {
                while (!FindWindow())
                {
                    await Task.Delay(200);
                }

                // Re-parent the child window
                SetParent(hwndControl, hwndHost);
            });

            // Start the external application if specified
            if (appPath.Length > 0)
            {
                Process.Start(appPath);
            }

            return new HandleRef(this, hwndHost);
        }

        protected override IntPtr WndProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            // Nothing to do here...
            handled = false;
            return IntPtr.Zero;
        }

        protected override void DestroyWindowCore(HandleRef hwnd)
        {
            DestroyWindow(hwnd.Handle);
        }

        private bool FindWindow()
        {
            // Look for the specified Window name
            hwndControl = (IntPtr)FindWindow(null, windowName);
            if (hwndControl != IntPtr.Zero) return true;

            // If not found, try the class
            hwndControl = (IntPtr)FindWindow(windowName, null);
            if (hwndControl != IntPtr.Zero) return true;

            return false;
        }
    }
}
