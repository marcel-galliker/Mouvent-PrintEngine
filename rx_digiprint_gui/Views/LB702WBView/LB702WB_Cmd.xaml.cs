using RX_DigiPrint.Models;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.LB702WBView
{
    /// <summary>
    /// Interaction logic for TexCmd.xaml
    /// </summary>
    public partial class LB702WB_Cmd : UserControl
    {
        public LB702WB_Cmd()
        {
            InitializeComponent();
            DataContext = RxGlobals.Plc;
        }
        
    }
}
