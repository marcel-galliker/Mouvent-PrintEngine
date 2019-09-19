using RX_DigiPrint.Models;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.CleafView
{
    /// <summary>
    /// Interaction logic for CleafCommands.xaml
    /// </summary>
    public partial class CleafCommands : UserControl
    {
        public CleafCommands()
        {
            InitializeComponent();

            DataContext = RxGlobals.Plc;
            RxGlobals.Timer.TimerFct += Timer;
        }
        
        //--- Timer ---------------------------------------------
        private void Timer(int no)
        {            
            if (!RxGlobals.Plc.Connected) return;
            string str;
            str = "Application.GUI_00_001_Main"+"\n"
                + "STA_REFERENCE_ENABLE"+"\n";

            RxGlobals.Plc.RequestVar(str);
        }
    }
}
