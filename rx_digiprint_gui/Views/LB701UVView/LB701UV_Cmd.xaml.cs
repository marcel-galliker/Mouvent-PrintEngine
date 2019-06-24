using RX_DigiPrint.Models;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.LB701UVView
{
    /// <summary>
    /// Interaction logic for TexCmd.xaml
    /// </summary>
    public partial class LB701UV_Cmd : UserControl
    {
        public LB701UV_Cmd()
        {
            InitializeComponent();
            DataContext = RxGlobals.Plc;
        }
        
    }
}
