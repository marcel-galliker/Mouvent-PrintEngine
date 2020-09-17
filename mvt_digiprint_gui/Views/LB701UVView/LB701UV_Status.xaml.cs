using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.LB701UVView
{
    /// <summary>
    /// Interaction logic for MainStatus.xaml
    /// </summary>
    public partial class LB701UV_Status : UserControl
    {
        public LB701UV_Status()
        {
            InitializeComponent();
            DataContext = this;

            CB_MainState.ItemsSource  = new EN_MachineStateList();
            StatusPanel.Update       += MlPrepare.Update;
            StatusPanel.Update       += StatusPanel_Update;
        }

        //--- StatusPanel_Update -------------------
        void StatusPanel_Update()
        {
            double timer  = Rx.StrToDouble(RxGlobals.Plc.GetVar(StatusPanel.UnitID, "STA_UV_LAMP_1_TIMER"));
            string ready  = RxGlobals.Plc.GetVar(StatusPanel.UnitID, "STA_UV_LAMPS_READY");

            if      (timer>0)               CuringLamp.Text = string.Format("{0} ON",  timer);
            else if (timer<0)               CuringLamp.Text = string.Format("{0} OFF", -timer);
            else if (ready==null)           CuringLamp.Text = null;
            else if (ready.Equals("TRUE"))  CuringLamp.Text = "ON";
            else                            CuringLamp.Text = "OFF";
        }
    }
}
