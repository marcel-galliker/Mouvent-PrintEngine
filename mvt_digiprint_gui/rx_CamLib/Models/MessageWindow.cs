using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using System.Security.Permissions;
using System.Text;
using System.Windows.Forms;


namespace rx_CamLib.Models
{
    class MessageWindow : NativeWindow
    {
        private readonly string _WindowName;
        private const int WmClose = 0x0010;

        public delegate void MessageCallBack(ref Message m);
        public event MessageCallBack MsgCallBack = null;

        [SuppressMessage("Microsoft.Reliability", "CA2006:UseSafeHandleToEncapsulateNativeResources")]
        private static readonly HandleRef HwndMessage = new HandleRef(null, new IntPtr(-3));

        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        [ResourceExposure(ResourceScope.None)]
        private static extern IntPtr PostMessage(HandleRef hwnd, int msg, int wparam, int lparam);

        [DllImport("user32.dll", ExactSpelling = true, CharSet = CharSet.Auto)]
        [ResourceExposure(ResourceScope.Process)]
        private static extern int GetWindowThreadProcessId(HandleRef hWnd, out int lpdwProcessId);

        [DllImport("kernel32.dll", ExactSpelling = true, CharSet = CharSet.Auto)]
        [ResourceExposure(ResourceScope.Process)]
        private static extern int GetCurrentThreadId();

        public MessageWindow(string caption)
        {
            _WindowName = caption;
        }

        [SecurityPermission(SecurityAction.Demand, Flags = SecurityPermissionFlag.UnmanagedCode)]
        protected override void WndProc(ref Message m)
        {
            // Listen for operating system messages
            MsgCallBack?.Invoke(ref m);
            base.WndProc(ref m);
        }

        public bool CreateWindow()
        {
            if (Handle == IntPtr.Zero)
            {
                CreateHandle(new CreateParams
                {
                    Style = 0,
                    ExStyle = 0,
                    ClassStyle = 0,
                    Caption = _WindowName,
                    Parent = (IntPtr)HwndMessage
                });
            }
            return Handle != IntPtr.Zero;
        }

        public void DestroyWindow()
        {
            DestroyWindow(true, IntPtr.Zero);
        }

        private bool GetInvokeRequired(IntPtr hWnd)
        {
            if (hWnd == IntPtr.Zero) return false;
            int pid;
            var hwndThread = GetWindowThreadProcessId(new HandleRef(this, hWnd), out pid);
            var currentThread = GetCurrentThreadId();
            return (hwndThread != currentThread);
        }

        private void DestroyWindow(bool destroyHwnd, IntPtr hWnd)
        {
            if (hWnd == IntPtr.Zero)
            {
                hWnd = Handle;
            }

            if (GetInvokeRequired(hWnd))
            {
                PostMessage(new HandleRef(this, hWnd), WmClose, 0, 0);
                return;
            }

            lock (this)
            {
                if (destroyHwnd)
                {
                    base.DestroyHandle();
                }
            }
        }

        public override void DestroyHandle()
        {
            DestroyWindow(false, IntPtr.Zero);
            base.DestroyHandle();
        }
    }
}
