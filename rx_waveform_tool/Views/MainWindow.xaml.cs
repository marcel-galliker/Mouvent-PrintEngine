using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using RxWfTool.Models;
using RxWfTool.Views;

namespace RxWfTool
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        WfDef   _WfDef = new WfDef();
        
        //--- Constructor --------------------------------------------------
        public MainWindow()
        {
            InitializeComponent();
            WfView.WfDef = _WfDef;
            Title = _WfDef.FileName;
        }

       //--- Open_Clicked -------------------------------------------------
        private void Open_Clicked(object sender, RoutedEventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Filter = string.Format("WaveForm definition files (*.{0})|*.{0}|All files (*.*)|*.*", WfDef.FileExtension);
            dlg.ShowDialog();
            if (dlg.FileName!="")
            { 
                _WfDef.open(dlg.FileName);
                WfView.WfDef = _WfDef;
                Title = _WfDef.FileName;
            }
        }
        
        //--- Save_Clicked -------------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            Dummy.Focus();
            _WfDef.save(_WfDef.FileName);
        }

        //--- SaveAs_Clicked -------------------------------------------------
        private void SaveAs_Clicked(object sender, RoutedEventArgs e)
        {
            SaveFileDialog  dlg = new SaveFileDialog();
            dlg.Filter = string.Format("WaveForm definition files (*.{0})|*.{0}|All files (*.*)|*.*", WfDef.FileExtension);
            dlg.DefaultExt=WfDef.FileExtension;
            dlg.ShowDialog();
            if (dlg.FileName!="")
                _WfDef.save(dlg.FileName);
            Title = _WfDef.FileName;
        }

        //--- Import_Clicked -------------------------------------------------
        private void Import_Clicked(object sender, RoutedEventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Filter = "WaveForm definition files (*.csv)|*.csv|All files (*.*)|*.*" ;
            dlg.ShowDialog();
            if (dlg.FileName!="")
            {
                _WfDef.import(dlg.FileName);      
                WfView.WfDef = _WfDef;
            }
        }

        //--- Exit_Clicked -------------------------------------------------
        private void Exit_Clicked(object sender, RoutedEventArgs e)
        {
            Close();
        }

        //--- Window_Closing ----------------------------------------------------
        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (_WfDef.Changed && MessageBox.Show("WaveForm has changed. Save it?", "Exit", MessageBoxButton.YesNo)==MessageBoxResult.Yes)
                _WfDef.save(_WfDef.FileName);
            Settings.RX_Settings.LastFile = _WfDef.FileName;
            Settings.RX_Settings.Width    = Convert.ToInt32(this.ActualWidth);
            Settings.RX_Settings.Height   = Convert.ToInt32(this.ActualHeight);
            Settings.RX_Settings.Save(Settings.Path);
        }

        //---About_Clicked -----------------------------------------------
        private void About_Clicked(object sender, RoutedEventArgs e)
        {
            Window wnd = new About();
            wnd.ShowDialog();
        }

        private void WfView_Loaded(object sender, RoutedEventArgs e)
        {
            Settings.RX_Settings.Load(Settings.Path);
            if (Settings.RX_Settings.Width >0) this.Width =Settings.RX_Settings.Width;
            if (Settings.RX_Settings.Height>0) this.Height=Settings.RX_Settings.Height;

            String[] args = Environment.GetCommandLineArgs();
            int err=1;
            if (args.Count()>1) err=_WfDef.open(args[1]);
         //   if (err!=0) _WfDef.open(Settings.RX_Settings.LastFile);
            WfView.WfDef = _WfDef;
            Title = _WfDef.FileName;
        }
    }
}
