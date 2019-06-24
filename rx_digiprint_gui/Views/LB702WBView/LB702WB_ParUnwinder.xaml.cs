using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Views.UserControls;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.LB702WBView
{
    /// <summary>
    /// Interaction logic for MainPar.xaml
    /// </summary>
    public partial class LB702WB_ParUnwinder : UserControl, IPlcParPanel
    {
        public LB702WB_ParUnwinder()
        {
            InitializeComponent();

            CB_RotUW.ItemsSource = CB_RotUW.ItemsSource = new EN_RotationList();
            CB_Corona.ItemsSource     = new EN_OnOff();
        }

        //--- Send -------------------------
        public void Send() {ParPanel.Send();}

        //--- Reset -------------------------
        public void Reset() {ParPanel.Reset();}
     }
}
