using Microsoft.Win32;
using RX_Common;
using RX_DigiPrint.Models.Enums;
using rx_rip_gui.Models;
using System.IO;
using System.Windows;
using System.Windows.Controls;

namespace rx_rip_gui.Views
{
    public partial class RippingView : UserControl
    {
        // DataContext : RippingCfg

        public RippingView()
        {
            InitializeComponent();

            CB_RenderIntent.ItemsSource     = new EN_RenderIntent();
            CB_BlackPreserve.ItemsSource    = new EN_BlackPreserve();
            CB_BlackPtComp.ItemsSource      = new EN_YesNo();
            CB_SimuOverprint.ItemsSource    = new EN_YesNo();
        }

        //--- Property Dir ---------------------------------------
        private string _Dir="";
        public string Dir
        {
            set { _Dir = value; }
        }

        //--- Outputprofile_Clicked ----------------------------------
        private void Outputprofile_Clicked(object sender, RoutedEventArgs e)
        {
            RippingCfg  cfg=DataContext as RippingCfg;


            OpenFileDialog dlg = new OpenFileDialog();

            dlg.Filter = "ICC files (*.icc)|*.icc|All files (*.*)|*.*" ;
            dlg.FilterIndex = 1 ;
            dlg.Multiselect = false;
            dlg.RestoreDirectory = true ;

            if(cfg!=null && dlg.ShowDialog()==true)
            { 
                cfg.OutputProfileName = System.IO.Path.GetFileName(dlg.FileName);
                string path=_Dir+"/"+cfg.OutputProfileName;
                File.Copy(dlg.FileName, path);
            }
        }

        //--- EditOutputprofile_Clicked ----------------------------------
        private void EditOutputprofile_Clicked(object sender, RoutedEventArgs e)
        {
            bool res=RxMessageBox.YesNo("Edit", "Not implemented yet.", MessageBoxImage.Asterisk, true);
        }
    }
}
