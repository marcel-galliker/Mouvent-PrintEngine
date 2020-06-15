using RX_Common;
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

namespace RX_DigiPrint.Views.TexView
{
    /// <summary>
    /// Interaction logic for MainPar.xaml
    /// </summary>
    public partial class TexCfg : UserControl
    {
        public TexCfg()
        {
            InitializeComponent();
            CB_SpreadRoller.ItemsSource  = new EN_SpreadRollerList();
            CB_BeltHeaterOn.ItemsSource  = new EN_OnOff();
        }

        //--- UserControl_IsVisibleChanged -----------------------------------
        private void UserControl_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue) RxGlobals.Timer.TimerFct += _Tick;
            else                  RxGlobals.Timer.TimerFct -= _Tick;
        }

        //--- _Tick --------------------------
        private EN_MachineStateList _states = new EN_MachineStateList();
        private void _Tick(int no)
        {
            Visibility visible=Visibility.Visible;
            try
            {
                string str = string.Format("{0}\n{1}\n", CfgPanel.UnitID, "STA_MACHINE_STATE");
                // RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_GET_VAR, str.ToString());
                RxGlobals.Plc.RequestVar(str.ToString());
                string state=_states.GetDisplay(Convert.ToInt32(RxGlobals.Plc.GetVar(CfgPanel.UnitID, "STA_MACHINE_STATE")));
                if (state.Equals("ERROR")) visible = Visibility.Collapsed;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
            HeadRefEnable.Visibility = visible; 
            MagneticBandEnable.Visibility = visible;
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

        //--- Reference_Clicked --------------------------------------
        private void Reference_Clicked(object sender, MouseButtonEventArgs e)
        {
            HeadReference.Send(CfgPanel);
            CfgPanel.Reset();
        }

        //--- Offset_Clicked --------------------------------------
        private void Offset_Clicked(object sender, MouseButtonEventArgs e)
        {
            SlaveOffset.Send(CfgPanel);
            CfgPanel.Reset();
        }
    }
}
