using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.UserControls;
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
    /// Interaction logic for TexConsumables.xaml
    /// </summary>
    public partial class TexConsumables : UserControl
    {
        public TexConsumables()
        {
            InitializeComponent();
         //   Panel.Update += Panel_Update;
         //   Panel_Update();
         //   Consumables.UnwinderMax = 400;
         //   Consumables.RewinderMax = 400;
        }

        //--- Panel_Update ---------------------------------------
        /*
        void Panel_Update()
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_FLUID_STAT);

            string unwinder_str = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_WINDER_DIAMETER_1");
            string rewinder_str = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_WINDER_DIAMETER_2");

            Consumables.Unwinder = Rx.StrToUInt32(unwinder_str);
            Consumables.Rewinder = Rx.StrToUInt32(rewinder_str);
        }
        */
    }
}
