using RX_DigiPrint.Models;
using RX_DigiPrint.Views.UserControls;
using System.Windows;
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
            ParPanel.PropertyChanged +=ParPanel_PropertyChanged;
        }

        //--- ParPanel_PropertyChanged --------------------
        void ParPanel_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
 	        if (e.PropertyName.Equals("Changed")) RxGlobals.LB702WB_Machine.Changed(4, ParPanel.Changed);
        }

        //--- Send -------------------------
        public void Send() {ParPanel.Send();}

        //--- Reset -------------------------
        public void Reset() {ParPanel.Reset();}
                
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
