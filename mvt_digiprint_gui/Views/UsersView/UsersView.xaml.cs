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

namespace RX_DigiPrint.Views.UsersView
{
    /// <summary>
    /// Main view for the Users Tab
    /// </summary>
    public partial class UsersView : UserControl
    {
        private readonly MvtUserLevelManager ulm = new MvtUserLevelManager(SettingsDlg.GetVersion(), RxGlobals.PrinterProperties.Host_Name);

        public UsersView()
        {
            InitializeComponent();

            UpdateView();
        }

        private void AddUser_clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            Window addUser = new AddUser();
            addUser.Owner = Window.GetWindow(this);
            addUser.ShowDialog();
            UpdateView();
        }

        private void RemoveUser_clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            Helpers.User s = GetSelectedUser();
            if(s != null)
            {
                ulm.RemoveUser(s.Username);
                UpdateView();
            }
        }

        private void ChangeValidity_clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            Helpers.User s = GetSelectedUser();
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
            Helpers.User s = GetSelectedUser();
            if (s != null)
            {
                QRCodeView qrCodeView = new QRCodeView(s.Username);
                qrCodeView.Owner = Window.GetWindow(this);
                qrCodeView.ShowDialog();
            }
        }

        /// <summary>
        /// Helper function to check if a User is selected
        /// </summary>
        /// <returns>true if a User is selected, false otherwise</returns>
        private bool UserSelected()
        {
            foreach (Row row in UsersGrid.Rows)
            {
                if (row.IsSelected) return true;
            }
            return false;
        }

        /// <summary>
        /// Helper function to get the selected User
        /// </summary>
        /// <returns>The selected User, or null if there isn't one</returns>
        private Helpers.User GetSelectedUser()
        {
            if(UserSelected())
            {
                foreach (Row row in UsersGrid.Rows)
                {
                    if (row.IsSelected)
                    {
                        return row.Data as Helpers.User;
                    }                    
                }
            }
            return null;
        }

        /// <summary>
        /// Helper function to update the DataGrid view containing the User list
        /// </summary>
        private void UpdateView()
        {
            UsersGrid.ItemsSource = ulm.GetAllUsers();
            UpdateButtons();
        }

        private void UpdateButtons()
        {
            if(UserSelected())
            {
                RemoveUser.Visibility = Visibility.Visible;
                ChangeValidity.Visibility = Visibility.Visible;
                ShowCode.Visibility = Visibility.Visible;
            }
            else
            {
                RemoveUser.Visibility = Visibility.Hidden;
                ChangeValidity.Visibility = Visibility.Hidden;
                ShowCode.Visibility = Visibility.Hidden;
            }
        }

        private void UsersGrid_SelectedItemsChanged(object sender, Infragistics.Windows.DataPresenter.Events.SelectedItemsChangedEventArgs e)
        {
            UpdateButtons();
        }

        private void UsersGrid_SelectedRowsCollectionChanged(object sender, Infragistics.Controls.Grids.SelectionCollectionChangedEventArgs<Infragistics.Controls.Grids.SelectedRowsCollection> e)
        {
            UpdateButtons();
        }
    }
}
