using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.UserControls;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace RX_DigiPrint.Views.LB702WBView
{
    /// <summary>
    /// Interaction logic for MainPar.xaml
    /// </summary>
    public partial class LB702WB_ParFlexo : UserControl, IPlcParPanel
    {
        private int _no;

        public LB702WB_ParFlexo()
        {
            InitializeComponent();
            CB_Mode.ItemsSource = new EN_OnOffAuto();

            ParPanel.PropertyChanged += ParPanel_PropertyChanged;
        }

        //--- ParPanel_PropertyChanged ----------------------------------
        void ParPanel_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Changed"))
            {
                switch (_no)
                {
                    case 1: RxGlobals.LB702WB_Machine.Changed(1, ParPanel.Changed); break;
                    case 2: RxGlobals.LB702WB_Machine.Changed(6, ParPanel.Changed); break;
                }
            }
        }

        //--- FlexoName ----------------------
        public string FlexoName
        {
            set
            {
                _no = Rx.StrToInt32(value);
                GroupBox.Header = "Flexo " + value;
                string name = value.ToUpper();
                for (int i = 0; i < ParPanel.Children.Count; i++)
                {
                    PlcParCtrl ctrl = ParPanel.Children[i] as PlcParCtrl;
                    if (ctrl != null) ctrl.ID = ctrl.ID.Replace("_1", "_" + name);
                }
            }
        }

        //--- Send -------------------------
        public void Send() { ParPanel.Send(); }

        //--- Reset -------------------------
        public void Reset() { ParPanel.Reset(); }

        //--- Save_Clicked ---------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            Send();
        }

        //--- Reload_Clicked ---------------------------------------------
        private void Reload_Clicked(object sender, RoutedEventArgs e)
        {
            Reset();
        }
    }
}
