using RX_DigiPrint.External;
using RX_DigiPrint.Models;
using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace RX_DigiPrint.Views.Settings
{
    /// <summary>
    /// Interaction logic for NetworkItem.xaml
    /// </summary>
    public partial class NetworkItem : UserControl
    {        
        public Guid InterfaceGuid;

        public NetworkItem()
        {
            // WlanNetwork DataContext;
            InitializeComponent();
        }

        //--- UserControl_MouseEnter -----------------------------
        private void UserControl_MouseEnter(object sender, MouseEventArgs e)
        {
            Background = (Brush)App.Current.Resources["Button.MouseOver.Background"];
        }

        //--- UserControl_MouseLeave -----------------------------
        private void UserControl_MouseLeave(object sender, MouseEventArgs e)
        {
            Background = Brushes.Transparent;
        }

        //--- UserControl_MouseLeftButtonUp -----------------------------------
        private void UserControl_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            WlanNetwork net = DataContext as WlanNetwork;
            if (net==null) return;
            if (net.HasProfile) 
            {
                RX_Wlan.Connect(InterfaceGuid, net.Name);
                if (RxGlobals.Popup!=null) RxGlobals.Popup.IsOpen=false;
            }
            else
            {
                Password.Text="";
                MainGrid.RowDefinitions[1].Height=GridLength.Auto;
                Password.OpenPad();
            }
        }

        //--- Password_LostFocus ---------------------------------------
        private void Password_LostFocus(object sender, RoutedEventArgs e)
        {
            WlanNetwork net = DataContext as WlanNetwork;
            if (net==null) return;
           // RX_Wlan.set_profile(InterfaceGuid, "Test", "Password");
            RX_Wlan.set_profile(InterfaceGuid, net.Name, Password.Text);
            RX_Wlan.Connect(InterfaceGuid, net.Name);
        }

    }
}
