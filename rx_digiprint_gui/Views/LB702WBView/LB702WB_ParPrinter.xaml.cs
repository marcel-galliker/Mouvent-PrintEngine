using RX_DigiPrint.Views.UserControls;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.LB702WBView
{
    /// <summary>
    /// Interaction logic for MainPar.xaml
    /// </summary>
    public partial class LB702WB_ParPrinter : UserControl, IPlcParPanel
    {
        public LB702WB_ParPrinter()
        {
            InitializeComponent();
        }

        //--- Send -------------------------
        public void Send() {ParPanel.Send();}

        //--- Reset -------------------------
        public void Reset() {ParPanel.Reset();}

     }
}
