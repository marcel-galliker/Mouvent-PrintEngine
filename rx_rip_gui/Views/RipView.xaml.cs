using RX_Common;
using rx_rip_gui.Models;
using System;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;

namespace rx_rip_gui.Views
{
    public partial class RipView: UserControl
    {
        public RipView()
        {
            InitializeComponent();
            PrEnvSettings.PrEnvAdd += PrEnvAdd;
            PrEnvSettings.PrEnvDelete += PrEnvDelete;
        }

        //--- PrEnvAdd ----------------------
        private void PrEnvAdd(PrEnv prenv)
        {
            (PrEnvList.ItemsSource as ObservableCollection<PrEnv>).Add(prenv);
            PrEnvList.ActiveItem = prenv;
            foreach(var row in PrEnvList.Rows)
                row.IsSelected = row.Data.Equals(prenv);
        }

        //--- PrEnvDelete ----------------------
        private void PrEnvDelete(PrEnv prenv)
        {
            (PrEnvList.ItemsSource as ObservableCollection<PrEnv>).Remove(prenv);
            foreach(var row in PrEnvList.Rows)
                row.IsSelected = false;
        }

        //--- Property ItemsSource ---------------------------------------
        public ObservableCollection<PrEnv> ItemsSource
        {
            set { PrEnvList.ItemsSource = value; }
        }
    
        //--- XamGrid_SelectedRowsCollectionChanged ---------------------------------------
        private void XamGrid_SelectedRowsCollectionChanged(object sender, Infragistics.Controls.Grids.SelectionCollectionChangedEventArgs<Infragistics.Controls.Grids.SelectedRowsCollection> e)
        {
            if (e.PreviouslySelectedItems.Count>0)
            {
                PrEnv item = e.PreviouslySelectedItems[0].Data as PrEnv;
                if (item!=null && item.Changed)
                {
                    if (RxMessageBox.YesNo("Save", string.Format("Print Environment {0} has changed. Save changes?", item.Name), MessageBoxImage.Question, true))
                        item.Save();
                }
            }

            if (e.NewSelectedItems.Count==0) PrEnvSettings.PrEnv = null;
            else                             PrEnvSettings.PrEnv = e.NewSelectedItems[0].Data as PrEnv;
        }
    }
}
