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
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for PlcCommand.xaml
    /// </summary>
    public partial class PlcCommand : UserControl
    {
        public PlcCommand()
        {
            InitializeComponent();
//            DataContext = this;
        }

        //--- Property ID ---------------------------------------
        private string _ID;
        public string ID    
        {
            get { return _ID; }
            set { _ID = value; }
        }

        //--- Property Label ---------------------------------------
        public string Label
        {
            get { return ButtonCtrl.Content.ToString(); }
            set { ButtonCtrl.Content = value; }
        }

        //--- Property User ---------------------------------------
        private EUserType _User;
        public EUserType User
        {
            get { return _User; }
            set
            { 
                RxGlobals.User.PropertyChanged += User_PropertyChanged;
                _User = value;
                _setUserType();
            }
        }

        //--- _setUserType ---------------
        private void _setUserType()
        {
            Visibility = (RxGlobals.User.UserType>=User) ? Visibility.Visible : Visibility.Collapsed;
        }

        //--- User_PropertyChanged ----------------------------
        void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            _setUserType();            
        }
        
        //--- Property CheckButton ---------------------------------------
        private bool _CheckButton=false;
        public bool CheckButton
        {
            get { return _CheckButton; }
            set { _CheckButton = value; }
        }

        //--- Property Confirm ---------------------------------------
        private bool _Confirm=false;
        public bool Confirm
        {
            get { return _Confirm; }
            set { _Confirm = value; }
        }
        
        //--- ButtonCtrl_Click --------------------------------------
        private void ButtonCtrl_Click(object sender, RoutedEventArgs e)
        {
            if (!_CheckButton) ButtonCtrl.IsChecked = false;
            int val=(!_CheckButton || _CheckButton&&ButtonCtrl.IsChecked==true) ? 1:0;

            e.Handled = true; 
            
            if (val!=0 && Confirm)
            {
                // ask and return of not "YES"
                if (RxMessageBox.YesNo(Label, string.Format("Execute {0} ?", Label.ToUpper()),  MessageBoxImage.Question, false))
                    return;
            }
            Console.WriteLine("Command >>{0}<<", ID);
            RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_CMD, ID);
        }
    }
}
