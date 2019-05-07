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
        public LB702WB_ParDryer()
        {
            InitializeComponent();

            CB_Mode.ItemsSource  = new EN_OnOffAuto();
        }

        //--- DryerName ----------------------
        public string DryerName 
        { 
            set 
            {
                GroupBox.Header = "Dryer "+value;
                string name = value.ToUpper();
                for (int i=0; i<ParPanel.Children.Count; i++)
                {
                    PlcParCtrl ctrl = ParPanel.Children[i] as PlcParCtrl;
                    if (ctrl!=null) ctrl.ID = ctrl.ID.Replace("DRYER_1", "DRYER_"+name);
                }
            } 
        }

        //--- Send -------------------------
        public void Send() {ParPanel.Send();}

        //--- Reset -------------------------
        public void Reset() {ParPanel.Reset();}
    }
}
