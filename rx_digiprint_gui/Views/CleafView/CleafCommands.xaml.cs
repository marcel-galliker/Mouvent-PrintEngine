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
        }
    }
}
