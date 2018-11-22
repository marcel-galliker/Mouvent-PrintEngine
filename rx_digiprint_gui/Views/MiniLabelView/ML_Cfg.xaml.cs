using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace RX_DigiPrint.Views.MiniLabelView
{
    /// <summary>
    /// Interaction logic for MainPar.xaml
    /// </summary>
    public partial class ML_Cfg : UserControl
    {
        public ML_Cfg()
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
       //    CfgPanel.Editing=true;
        }
    }
}
