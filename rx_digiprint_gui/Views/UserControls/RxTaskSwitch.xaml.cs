using RX_Common;
using RX_DigiPrint.Models;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for RxTaskSwitch.xaml
    /// </summary>
    public partial class RxTaskSwitch : Window
    {

        public RxTaskSwitch()
        {
            InitializeComponent();
            Visibility = Visibility.Collapsed;
        }

        //--- Window_Loaded ------------------------------------------------------------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            Rx.wnd_remove_close_button(this);
            Left=(RxScreen.Screen.Width-ActualWidth)/2*RxScreen.Screen.Scale;
        }

        //--- _update_tasklist ----------------------------------------------------------------
        private void _update_tasklist()
        {
            int i=0;
            foreach(RxTaskButton b in TaskList.Children)
            {
                b.used=0;
            //    Console.WriteLine("Task[{0}]: {1}.{2} handle={4} used={3}", i++, b.Process.ProcessName, b.Process.MainWindowTitle, b.used, b.Process.MainWindowHandle.ToString());
            }
            foreach (Process p in Process.GetProcesses())
            {
                string image="";
                string title=p.MainWindowTitle;
                int order=0;

                Console.WriteLine("ProcessName: >>{0}<< Title=>>{1}<<", p.ProcessName, p.MainWindowTitle);

                if      (p.ProcessName.Equals("RX_DigiPrint"))  {image="..\\..\\mouvent.ico"; order=1; title="GUI";}
                else if (p.ProcessName.Equals("putty"))         
                {
                    image="putty.png";
                    title=RxGlobals.Network.GetWindowTitle(p.MainWindowTitle);
                    order=2; 
                }
                else if (p.ProcessName.Equals("WinSCP"))        {image="WinSCP.ico";          order=3; title="WinSCP";}

                if (!image.Equals(""))
                {
                    bool found=false;

                    foreach(RxTaskButton b in TaskList.Children)
                    {
                        if (b.Process!=null && b.Process.MainWindowHandle == p.MainWindowHandle) 
                        {
                            found=true;
                            b.used++;
                            break;
                        }
                    }
                    if (!found)
                    {
                        RxTaskButton b = new RxTaskButton();
                        b.Image=image;
                        b.Text=title;
                        b.Process=p;
                        b.Order = order;
                        b.used = 1;
                        b.Click += Button_Clicked;

                        for(i=0; i<TaskList.Children.Count; i++)
                        {   
                            if ((TaskList.Children[i] as RxTaskButton).Order>order) break;
                        }
                        TaskList.Children.Insert(i, b);
                    }
                }
            } 
            for(i=0; i<TaskList.Children.Count; i++)
            {
                RxTaskButton b = TaskList.Children[i] as RxTaskButton;
           //     Console.WriteLine("Task[{0}]: {1}.{2} handle={4} used={3}", i, b.Process.ProcessName, b.Process.MainWindowTitle, b.used, b.Process.MainWindowHandle.ToString());
                if (b.used==0) TaskList.Children.RemoveAt(i);
            }
        }

        //--- Button_Clicked --------------------------------------------------------------------------------
        [DllImport("USER32.DLL")]
        private static extern bool SetForegroundWindow(IntPtr hWnd);
        
        [DllImport("USER32.DLL", CharSet = CharSet.Unicode)]
        private static extern IntPtr FindWindow(String lpClassName, String lpWindowName);

        private void Button_Clicked(object sender, RoutedEventArgs e)
        {
            RxTaskButton b = sender as RxTaskButton;
            if (b.Text.Equals("GUI"))
            {
                string name=System.IO.Path.GetFileNameWithoutExtension(System.Windows.Forms.Application.ExecutablePath);
                // Get a handle to the Calculator application.
                IntPtr handle = FindWindow(null, name);

                // Verify that Calculator is a running process.
                if (handle == IntPtr.Zero) return;

                // Make Calculator the foreground application
                SetForegroundWindow(handle);
            }
            else SetForegroundWindow(b.Process.MainWindowHandle);
            _update_tasklist();
            Topmost=true;
        }

        //--- RxTaskButton ---------------------------------------------------------------
        private void Window_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue)
            {
                _update_tasklist();
                Topmost=true;
            }
        }
    }
}
