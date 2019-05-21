using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.LB702WBView
{
    /// <summary>
    /// Interaction logic for MainStatus.xaml
    /// </summary>
    public partial class LB702WB_Status : UserControl
    {
        public LB702WB_Status()
        {
            InitializeComponent();
            DataContext = this;

            CB_MainState.ItemsSource  = new EN_MachineStateList();
            StatusPanel.Update       += MlPrepare.Update;
        }
    }
}
