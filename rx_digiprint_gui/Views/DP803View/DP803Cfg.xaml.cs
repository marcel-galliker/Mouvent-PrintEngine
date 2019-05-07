using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
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

namespace RX_DigiPrint.Views.DP803View
{
    /// <summary>
    /// Interaction logic for MainPar.xaml
    /// </summary>
    public partial class DP803Cfg : UserControl
    {
        public DP803Cfg()
        {
            InitializeComponent();
        }

        //--- Save_Clicked ---------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
           CfgPanel.Send(); 
           RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_SAVE_PAR);
        }

        //--- Reload_Clicked ---------------------------------------------
        private void Reload_Clicked(object sender, RoutedEventArgs e)
        {
           CfgPanel.Reset();
        }

        //--- ParPanel_PreviewKeyDown --------------------------------
        private void ParPanel_PreviewKeyDown(object sender, KeyEventArgs e)
        {
        //   CfgPanel.Editing=true;
        }
    }
}
