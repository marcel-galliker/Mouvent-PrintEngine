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
using System.Windows.Shapes;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Views.Settings;

namespace RX_DigiPrint.Views.SupervisorsView
{
    /// <summary>
    /// Class managing the view to add a supervisor
    /// </summary>
    public partial class AddSupervisor : Window
    {
        private MvtUserLevelManager ulm;
        public AddSupervisor()
        {
            InitializeComponent();

            ulm = new MvtUserLevelManager(SettingsDlg.GetVersion(), RxGlobals.PrinterProperties.Host_Name);
            ulm.NewMessage += LogSupervisorAddition;
        }

        private void ValidateCreation_Click(object sender, RoutedEventArgs e)
        {
            /* If a name was entered and a Date chosen, Superisor is added and his QR code is showed */
            if(!String.IsNullOrWhiteSpace(Username.Text) && ValidityCal.SelectedDate != null)
            {
                ulm.AddSupervisor(Username.Text, (DateTime)ValidityCal.SelectedDate);
                QRCodeView qrCode = new QRCodeView(Username.Text);
                qrCode.Owner = Window.GetWindow(this);
                qrCode.ShowDialog();
                Close();
            }          
        }

        private void CancelCreation_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        public static void LogSupervisorAddition(String message)
        {
            RxGlobals.Events.AddItem(new LogItem(message));
        }

    }
}
