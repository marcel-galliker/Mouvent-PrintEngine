using Microsoft.Win32;
using RX_Common;
using RX_DigiPrint.Models;
using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.Text;
using System.Windows;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for SetComputerNameWindow.xaml
    /// </summary>
    public partial class SetComputerNameWindow : Window
    {
        public SetComputerNameWindow()
        {
            InitializeComponent();
        }

        //--- Window_Loaded ------------------------------------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            NewName.Text = RxGlobals.PrinterProperties.Host_Name+"-GUI";    
        }

        //--- SetMachineName ---------------
        private static bool SetMachineName(string newName)
        {
            string path=Path.GetTempFileName();
            StreamWriter file = new StreamWriter(path); 
            string reg = string.Format(
                "Windows Registry Editor Version 5.00\n"+
                "\n"+
                "[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\ComputerName]\n"+
                "ComputerName=\"{0}\"\n"+
                "\n"+
                "[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName]\n"+
                "ActiveComputerName=\"{0}\"\n"+
                "\n"+
                "[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters]\n"+
                "\"NV Hostname\"=\"{0}\"\n"+
                "\"Hostname\"=\"{0}\""
                , newName);
            file.WriteLine(reg);
            file.Close();
            string arg = string.Format("import \"{0}\"", path);
            int exitcode= Rx.ExecuteProcessAsAdmin("c:\\windows\\system32\\reg.exe", arg);
            File.Delete(path);
            return true;
        }

        //--- Save_Clicked ---------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            string name = Rx.GetComputerName();
            if (name!=null)
            {
                if ( SetMachineName(NewName.Text) && RxMessageBox.YesNo("Restart", "Restarting the GUI!",  MessageBoxImage.Question, false))
                { 
                    Rx.StartProcess("shutdown", "/r /t 0");
                }
            }
        }

        //--- Cancel_Clicked ---------------------------------------------
        private void Cancel_Clicked(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
