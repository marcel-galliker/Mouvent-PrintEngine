using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.TestTableView
{
    /// <summary>
    /// Interaction logic for TestTableCommands.xaml
    /// </summary>
    public partial class TestTableCommands : UserControl
    {
        public TestTableCommands()
        {
            InitializeComponent();
            DataContext = RxGlobals.StepperStatus;

			RxGlobals.StepperStatus[0].PropertyChanged +=TestTableCommands_PropertyChanged;
        }

		private void TestTableCommands_PropertyChanged(object sender,System.ComponentModel.PropertyChangedEventArgs e)
		{
			if (e.PropertyName.Equals("CmdRunning"))
		    { 
                if (RxGlobals.StepperStatus[0].CmdRunning==0)
                {
                    Button_Stop.IsChecked=false;
                    Button_Reference.IsChecked=false;
                    Button_ScanRight.IsChecked=false;
                    Button_ScanLeft.IsChecked=false;
                }
			}
		}

		//--- Stop_Clicked -------------------------------------------------
		private void Stop_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TT_STOP);
        }

        //--- StartRef_Clicked -------------------------------------------------
        private void StartRef_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TT_START_REF);  
        }

        //--- ScanRight_Clicked -------------------------------------------------
        private void ScanRight_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TT_SCAN_RIGHT);  
        }

        //--- ScanLeft_Clicked -------------------------------------------------
        private void ScanLeft_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_TT_SCAN_LEFT);  
        }
    }
}
