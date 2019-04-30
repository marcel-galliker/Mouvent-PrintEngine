using RX_DigiPrint.Models;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.MiniLabelView
{
    /// <summary>
    /// Interaction logic for TexCmd.xaml
    /// </summary>
    public partial class ML_Cmd : UserControl
    {
        public ML_Cmd()
        {
            InitializeComponent();
            DataContext = RxGlobals.Plc;
        }
        
    }
}
