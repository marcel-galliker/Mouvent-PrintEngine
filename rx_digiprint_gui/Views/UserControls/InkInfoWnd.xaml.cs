using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
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

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for InkInfoWnd.xaml
    /// </summary>
    public partial class InkInfoWnd : Window
    {
        private InkSupply _InkSupply;
        private bool      _Identify=false;

        public InkInfoWnd(InkSupply inkSupply)
        {
            InitializeComponent();
            _InkSupply = inkSupply;
            DataContext = _InkSupply;
            MsgTitle.Text = _InkSupply.InkType.Name;
            Level.Enabled = false;
            Button_Candel.Focus();
            if (RxGlobals.User.UserType<EUserType.usr_service)
            {
                Tara_button.Visibility = Init_Button.Visibility = Visibility.Collapsed;
            }
        }

        //---Tara_Clicked ----------------------------------------
        private void Tara_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxMessageBox.YesNo("Set TARA", string.Format("Set Tara for {0}?", Title),  MessageBoxImage.Question, false))
            {
                TcpIp.SValue msg = new TcpIp.SValue(){no=_InkSupply.No-1};
                RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SCALES_TARA, ref msg);
            }
        }

        //---Reset_Clicked ----------------------------------------
        private void Reset_Clicked(object sender, RoutedEventArgs e)
        {
            TcpIp.SValue msg = new TcpIp.SValue(){no=_InkSupply.No-1};
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_BCSCANNER_RESET, ref msg);
        }

        //---Init_Clicked ----------------------------------------
        private void Init_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxMessageBox.YesNo("Init Barcode Scanner", string.Format("Init Barcode Scanner {0}?", Title),  MessageBoxImage.Question, false))
            {
                _Identify = true;
                TcpIp.SValue msg = new TcpIp.SValue(){no=_InkSupply.No-1};
                RxGlobals.RxInterface.SendMsg(TcpIp.CMD_BCSCANNER_IDENTIFY, ref msg);
            }
        }

        //---Trigger_Clicked ----------------------------------------
        private void Trigger_Clicked(object sender, RoutedEventArgs e)
        {
            TcpIp.SValue msg = new TcpIp.SValue(){no=_InkSupply.No-1};
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_BCSCANNER_TRIGGER, ref msg);
        }

        //---Cancel_Clicked ----------------------------------------
        private void Cancel_Clicked(object sender, RoutedEventArgs e)
        {
            if (_Identify)
            {
                TcpIp.SValue msg = new TcpIp.SValue(){no=-1};
                RxGlobals.RxInterface.SendMsg(TcpIp.CMD_BCSCANNER_IDENTIFY, ref msg);
                _Identify=false;
            }
            Close();
        }
    }
}
