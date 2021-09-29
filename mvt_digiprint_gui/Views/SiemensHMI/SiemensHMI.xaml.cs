using System.Windows.Controls;
using System;

namespace RX_DigiPrint.Views.SiemensHMI
{
    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class SiemensHMIView : UserControl
    {
        public SiemensHMIView()
        {
            InitializeComponent();

            String hmiWindowName = "SIMATIC WinCC Runtime Advanced";
            String hmiStarterExe = "C:\\Program Files (x86)\\Siemens\\Automation\\WinCC RT Advanced\\StartCenter.exe";            
            Win32ControlHost myControl = new Win32ControlHost(hmiWindowName, hmiStarterExe);

            this.AddChild(myControl);
            Loaded += delegate
            {
                myControl.Resize(this.ActualWidth, this.ActualHeight);
            };
        }
    }
}
