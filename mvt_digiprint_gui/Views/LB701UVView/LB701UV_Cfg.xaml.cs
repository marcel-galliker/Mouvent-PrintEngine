using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace RX_DigiPrint.Views.LB701UVView
{
    /// <summary>
    /// Interaction logic for MainPar.xaml
    /// </summary>
    public partial class LB701UV_Cfg : UserControl
    {
        private List<RxEnum<int>> _Dist=new List<RxEnum<int>>();

        public LB701UV_Cfg()
        {
            InitializeComponent();
                      
            _Dist.Add(new RxEnum<int>(240,  "240"));
            _Dist.Add(new RxEnum<int>(260,  "260"));
            CB_Dist.ItemsSource   = _Dist;
            CB_DieCut.ItemsSource = new EN_OnOff();  
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
