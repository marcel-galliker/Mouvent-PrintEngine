using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.LB702UVView
{
    /// <summary>
    /// Interaction logic for LB701UVView.xaml
    /// </summary>
    public partial class LB702UV_View : UserControl
    {
        public LB702UV_View()
        {
            InitializeComponent();
            PlcNotConnected.DataContext = RxGlobals.Plc;
            
            RB_Main.IsChecked=true;

            RxGlobals.User.PropertyChanged += User_PropertyChanged;
            User_PropertyChanged(null, null);
        }

        //--- User_PropertyChanged ----------------------------------
        private void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            Visibility visibility = (RxGlobals.User.UserType >= EUserType.usr_maintenance) ? Visibility.Visible : Visibility.Collapsed;
            RB_Log.Visibility = visibility;
            RB_Plc.Visibility = visibility;
        }
    }
}
