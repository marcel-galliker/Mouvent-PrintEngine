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
using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Views;
using Infragistics.Windows.DataPresenter;
using System.Globalization;
using System.IO;
using Infragistics.Controls.Grids;
using RX_DigiPrint.Views.Settings;

namespace RX_DigiPrint.Views.SupervisorsView
{
    /// <summary>
    /// Main view for the Supervisors Tab
    /// </summary>
    public partial class SupervisorsView : UserControl
    {
        private readonly MvtUserLevelManager ulm = new MvtUserLevelManager(SettingsDlg.GetVersion(), RxGlobals.PrinterProperties.Host_Name);

        public SupervisorsView()
        {
            InitializeComponent();

            UpdateView();
        }

        private void AddSupervisor_clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            Window addSupervisor = new AddSupervisor();
            addSupervisor.Owner = Window.GetWindow(this);
            addSupervisor.ShowDialog();
            UpdateView();
        }

        private void RemoveSupervisor_clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            Supervisor s = GetSelectedSupervisor();
            if(s != null)
            {
                ulm.RemoveSupervisor(s.Username);
                UpdateView();
            }
        }

        private void ChangeValidity_clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            Supervisor s = GetSelectedSupervisor();
            if (s != null)
            {
                CalendarValidity calendarValidity = new CalendarValidity(ulm, s.Username);
                calendarValidity.Owner = Window.GetWindow(this);
                calendarValidity.ShowDialog();
                UpdateView();
            }
        }

        private void ShowCode_Click(object sender, RoutedEventArgs e)
        {
            Supervisor s = GetSelectedSupervisor();
            if (s != null)
            {
                QRCodeView qrCodeView = new QRCodeView(s.Username);
                qrCodeView.Owner = Window.GetWindow(this);
                qrCodeView.ShowDialog();
            }
        }

        /// <summary>
        /// Helper function to check if a supervisor is selected
        /// </summary>
        /// <returns>true if a supervisor is selected, false otherwise</returns>
        private bool SupervisorSelected()
        {
            foreach (Row row in SupervisorsGrid.Rows)
            {
                if (row.IsSelected) return true;
            }
            return false;
        }

        /// <summary>
        /// Helper function to get the selected supervisor
        /// </summary>
        /// <returns>The selected supervisor, or null if there isn't one</returns>
        private Supervisor GetSelectedSupervisor()
        {
            if(SupervisorSelected())
            {
                foreach (Row row in SupervisorsGrid.Rows)
                {
                    if (row.IsSelected)
                    {
                        return row.Data as Supervisor;
                    }                    
                }
            }
            return null;
        }

        /// <summary>
        /// Helper function to update the DataGrid view containing the supervisor list
        /// </summary>
        private void UpdateView()
        {
            SupervisorsGrid.ItemsSource = ulm.GetAllSupervisors();
            UpdateButtons();
        }

        private void UpdateButtons()
        {
            if(SupervisorSelected())
            {
                RemoveSupervisor.Visibility = Visibility.Visible;
                ChangeValidity.Visibility = Visibility.Visible;
                ShowCode.Visibility = Visibility.Visible;
            }
            else
            {
                RemoveSupervisor.Visibility = Visibility.Hidden;
                ChangeValidity.Visibility = Visibility.Hidden;
                ShowCode.Visibility = Visibility.Hidden;
            }
        }

        private void SupervisorsGrid_SelectedItemsChanged(object sender, Infragistics.Windows.DataPresenter.Events.SelectedItemsChangedEventArgs e)
        {
            UpdateButtons();
        }

        private void SupervisorsGrid_SelectedRowsCollectionChanged(object sender, Infragistics.Controls.Grids.SelectionCollectionChangedEventArgs<Infragistics.Controls.Grids.SelectedRowsCollection> e)
        {
            UpdateButtons();
        }
    }
}
