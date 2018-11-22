using Microsoft.Win32;
using RX_Common;
using rx_rip_gui.Models;
using rx_rip_gui.Services;
using System;
using System.Windows;
using System.Windows.Controls;
using System.Xml;

namespace rx_rip_gui.Views
{
    public partial class SettingsView : UserControl
    {
        // DataContext = PrEnv
        private PrEnv _PrEnv;
        public delegate void PrEnvAddedDelegate(PrEnv prenv);
        public PrEnvAddedDelegate PrEnvAdd = null;
        public PrEnvAddedDelegate PrEnvDelete = null;

        //--- Constructor ------------------------------
        public SettingsView()
        {
            InitializeComponent();
            
            //--- create empty print environent ---
            DataContext = new PrEnv();

            _update_childs();
        }        

        //--- Property PrEnv ---------------------------------------
        public PrEnv PrEnv
        {
            set 
            { 
                _PrEnv =value;
                if (value!=null) DataContext = _PrEnv.Clone();
                else             DataContext = new PrEnv();
            }
        }        

        //--- _update_childs ---------------------------------
        private void _update_childs()
        {
            PrEnv item = DataContext as PrEnv;
            if (item==null) 
            {
            //    ScreeningView.DataContext = null;
                RippingView.DataContext   = null;
            }
            else
            {
            //    ScreeningView.Visibility  = Visibility.Visible;
            //    ScreeningView.DataContext = item.Screening;
            //    ScreeningView.Dir         = item.Dir(null);
                RippingView.Visibility    = Visibility.Visible;
                RippingView.DataContext   = item.Ripping;
                RippingView.Dir           = item.Dir(null);
            }        
        }


        //--- UserControl_DataContextChanged ---------------------------
        private void UserControl_DataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            _update_childs();
        }

        //--- Add_Clicked ------------------------------------------
        private void Add_Clicked(object sender, RoutedEventArgs e)
        { 
            PrEnv  cfg=DataContext as PrEnv;
            if (cfg!=null)
            {
                RxTextPad pad = new RxTextPad(PrEnvName);
                if((bool)pad.ShowDialog())
                { 
                    PrEnv newPrEnv = (PrEnv)cfg.Clone();
                    newPrEnv.Name = pad.Result;
                    newPrEnv.Save();
                    if (PrEnvAdd!=null) PrEnvAdd(newPrEnv);
                }
            }
        }

        //--- Save_Clicked ------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            PrEnv item = DataContext as PrEnv;
            if (item!=null) 
            {
                _PrEnv = DataContext as PrEnv;
                item.Save();
            }
        }

        //--- Reload_Clicked ------------------------------------------
        private void Reload_Clicked(object sender, RoutedEventArgs e)
        { 
            if (_PrEnv!=null) DataContext = _PrEnv.Clone();
        }

        //--- Delete_Clicked ------------------------------------------
        private void Delete_Clicked(object sender, RoutedEventArgs e)
        { 
            PrEnv item = DataContext as PrEnv;
            if (item!=null && RxMessageBox.YesNo("Delete", string.Format("Delete the Print Environment {0}?", item.Name), MessageBoxImage.Question, true))
            {
                item.Delete();
                if (PrEnvDelete!=null) PrEnvDelete(_PrEnv);
            }
        }
    }
}
