using Microsoft.Win32;
using RX_Common;
using RX_DigiPrint.Models.Enums;
using rx_rip_gui.Models;
using System.IO;
using System.Windows;
using System.Windows.Controls;

namespace rx_rip_gui.Views
{
    public partial class ScreeningView : UserControl
    {
        // DataContext : ScreeningCfg
        public ScreeningView()
        {
            InitializeComponent();
            CB_ScreeningMode.ItemsSource = new EN_ScreeningMethod();
        }

        //--- Property Dir ---------------------------------------
        private string _Dir="";
        public string Dir
        {
            set { _Dir = value; }
        }

        //--- Profile_Clicked ----------------------------------
        private void Profile_Clicked(object sender, RoutedEventArgs e)
        {
            ScreeningCfg  cfg=DataContext as ScreeningCfg;

            OpenFileDialog dlg = new OpenFileDialog();

            dlg.Filter = "profile files (*.ted)|*.ted|All files (*.*)|*.*" ;
            dlg.FilterIndex = 1 ;
            dlg.Multiselect = false;
            dlg.RestoreDirectory = true ;

            if(cfg!=null && dlg.ShowDialog()==true)
            { 
                cfg.ProfileName = System.IO.Path.GetFileName(dlg.FileName);
                string path=_Dir+"/"+cfg.ProfileName;
                File.Copy(dlg.FileName, path, true);
            }
        }

        //--- EditProfile_Clicked ----------------------------------
        private void EditProfile_Clicked(object sender, RoutedEventArgs e)
        {
            bool res=RxMessageBox.YesNo("Edit", "Not implemented yet.", MessageBoxImage.Asterisk, true);
        }

    }
}
