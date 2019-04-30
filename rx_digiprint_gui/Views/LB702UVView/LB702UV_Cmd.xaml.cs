using RX_DigiPrint.Models;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.LB702UVView
{
    /// <summary>
    /// Interaction logic for TexCmd.xaml
    /// </summary>
    public partial class LB702UV_Cmd : UserControl
    {
        public LB702UV_Cmd()
        {
            InitializeComponent();
            DataContext = RxGlobals.Plc;
        }
        
    }
}
