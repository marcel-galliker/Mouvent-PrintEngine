using RX_Common;
using RX_DigiPrint.External;
using RX_DigiPrint.Models;
using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media.Imaging;

namespace RX_DigiPrint.Views.Settings
{
    /// <summary>
    /// Interaction logic for WlanSettings.xaml
    /// </summary>
    public partial class WlanSettings : UserControl
    {        
        private bool _connected = false;
        private Guid _interfaceGuid;

        //--- creator ----------------------------------------------
        public WlanSettings()
        {
            InitializeComponent();
        }

        //--- _update ---------------------------
        void _update()
        {
            int connected, i;
            ObservableCollection<RX_Wlan.WLAN_INTERFACE_INFO> interfaces = RX_Wlan.get_interfaces();
            if (interfaces!=null && interfaces.Count>0) _interfaceGuid = interfaces[0].InterfaceGuid;           
            ObservableCollection<WlanNetwork> networks                   = RX_Wlan.get_networks(_interfaceGuid, out connected);
            
            if (connected>=0) 
            {
                WlanNetwork.Text=networks[connected].Name;
                WlanStatus.Source = (BitmapImage)App.Current.Resources["tcp_online_ico"];
            }
            else              
            {
                WlanNetwork.Text="";
                WlanStatus.Source = (BitmapImage)App.Current.Resources["tcp_offline_ico"];
            }
            _connected = (connected>=0);

            NetworkList.Children.Clear();
            for (i=0; i<networks.Count(); i++)
            {
                NetworkItem item = new NetworkItem(){DataContext=networks[i]};
                item.InterfaceGuid = _interfaceGuid;
                NetworkList.Children.Add(item);
            }
        }

        //--- WlanStatus_MouseLeftButtonUp --------------------------------------------
        private void WlanStatus_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (_connected && RxMessageBox.YesNo("WLAN", "Disconnect.",  MessageBoxImage.Question, false))
            {
                 RX_Wlan.Disconnect(_interfaceGuid);
                 _update();
            }
        }

        //--- UserControl_IsVisibleChanged ----------------
        private void UserControl_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue)
            {
                _update();
            }
        }
    }
}
