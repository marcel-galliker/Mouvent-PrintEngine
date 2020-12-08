using System;
using System.Diagnostics;
using System.Windows;
using System.IO;
using System.Threading;
using System.Windows.Input;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Globalization;

namespace RX_DigiPrint
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        [DllImport("USER32.DLL", CharSet = CharSet.Unicode)]
        private static extern IntPtr FindWindow(String lpClassName, String lpWindowName);

        [DllImport("USER32.DLL")]
        private static extern bool SetForegroundWindow(IntPtr hWnd);

        //--- _bringToFront --------------------------
        private void _bringToFront(string title) 
        {
            // Get a handle to the Calculator application.
            IntPtr handle = FindWindow(null, title);

            // Verify that Calculator is a running process.
            if (handle == IntPtr.Zero) return;

            // Make Calculator the foreground application
            SetForegroundWindow(handle);
        }

         //--- App -----------------------------
        public App()
        {
                // set localization to en-US to ensure a good conversion of floating number...
                CultureInfo.CurrentCulture = new CultureInfo("en-US", false);
                string name = Path.GetFileNameWithoutExtension(System.Windows.Forms.Application.ExecutablePath);
                if (Process.GetProcessesByName(name).Length > 1)
                {
                    _bringToFront(name);
                    Application.Current.Shutdown();
                }

                Application.Current.DispatcherUnhandledException += _DispatcherUnhandledException;

                // problem with touch: https://msdn.microsoft.com/en-us/library/dd901337(v=vs.90).aspx
                DisableWPFTabletSupport();            
        }

        //--- DispatcherUnhandledException ---------------------------------
        private void _DispatcherUnhandledException(object sender, System.Windows.Threading.DispatcherUnhandledExceptionEventArgs e)
        {
            Console.WriteLine("Exception: " + e.Exception.Message);
            e.Handled = true;
        }

        //--- DisableWPFTabletSupport ---------------------------
        public static void DisableWPFTabletSupport()
        {
            // Get a collection of the tablet devices for this window.  
            TabletDeviceCollection devices = System.Windows.Input.Tablet.TabletDevices;

            if (devices.Count > 0)
            {   
                // Get the Type of InputManager.
                Type inputManagerType = typeof(System.Windows.Input.InputManager);
        
                // Call the StylusLogic method on the InputManager.Current instance.
                object stylusLogic = inputManagerType.InvokeMember("StylusLogic",
                            BindingFlags.GetProperty | BindingFlags.Instance | BindingFlags.NonPublic,
                            null, InputManager.Current, null);

                if (stylusLogic != null)
                {
                    //  Get the type of the stylusLogic returned from the call to StylusLogic.
                    Type stylusLogicType = stylusLogic.GetType();
            
                    // Loop until there are no more devices to remove.
                    while (devices.Count > 0)
                    {
                        // Remove the first tablet device in the devices collection.
                        stylusLogicType.InvokeMember("OnTabletRemoved",
                                BindingFlags.InvokeMethod | BindingFlags.Instance | BindingFlags.NonPublic,
                                null, stylusLogic, new object[] { (uint)0 });
                    }                
                }
               
            }
        }
    }
}
