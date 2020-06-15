using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.UserControls;
using System;
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
    public partial class LB702WB_ParDryer : UserControl, IPlcParPanel
    {
        private int _no;

        public LB702WB_ParDryer()
        {
            InitializeComponent();

            CB_Mode.ItemsSource  = new EN_OnOffAuto();
            ParPanel.PropertyChanged +=ParPanel_PropertyChanged;
        }

        //--- ParPanel_PropertyChanged ----------------------------------
        void ParPanel_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
 	        if (e.PropertyName.Equals("Changed"))
            {
                switch(_no)
                {
                    case 1: RxGlobals.LB702WB_Machine.Changed(2, ParPanel.Changed); break;
                    case 2: RxGlobals.LB702WB_Machine.Changed(5, ParPanel.Changed); break;
                    case 3: RxGlobals.LB702WB_Machine.Changed(7, ParPanel.Changed); break;
                }
            }
        }

        //--- DryerName ----------------------
        public string DryerName 
        { 
            set 
            {
                _no = Rx.StrToInt32(value);
                Header.Text = "Dryer "+value;
                string name = value.ToUpper();
                for (int i=0; i<ParPanel.Children.Count; i++)
                {
                    PlcParCtrl ctrl = ParPanel.Children[i] as PlcParCtrl;
                    if (ctrl!=null) ctrl.ID = ctrl.ID.Replace("DRYER1", "DRYER"+name);
                }
            } 
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
