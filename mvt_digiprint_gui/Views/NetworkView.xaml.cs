using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;
using RX_DigiPrint.Models.Enums;
using RX_Common;
using System.IO;
using System.Threading;
using System.Runtime.InteropServices;
using System.Text;
using RX_DigiPrint.Views.UserControls;
using Infragistics.Controls.Grids;

namespace RX_DigiPrint.Views
{
    public partial class NetworkView : UserControl
    {
        private string _PuttyPath;

        //--- constructor --------------------------------------------------------
        public NetworkView()
        {
            InitializeComponent();

            DataContext = this;
            NetworkGrid.ItemsSource = RxGlobals.Network.List;
            
            _InitPutty();
            RxGlobals.User.PropertyChanged += User_PropertyChanged;
            User_PropertyChanged(null, null);

            #if !DEBUG
                if (!RxGlobals.Screen.Surface) Keyboard.Visibility = Visibility.Collapsed;
            #endif
        }        
       
        //--- _InitPutty ---------------------------------------------
        private void _InitPutty()
        {
            string  programsx86=System.Environment.GetFolderPath(System.Environment.SpecialFolder.ProgramFilesX86);
            string  programs   = programsx86.Remove(programsx86.Length-6);  // remove " (x86)"

            if      (File.Exists(programsx86+@"\Mouvent\putty.exe")) _PuttyPath = programsx86+@"\Mouvent\putty.exe";
            else if (File.Exists(programsx86+@"\putty\putty.exe"))   _PuttyPath = programsx86+@"\putty\putty.exe";
            else if (File.Exists(programs+@"\putty\putty.exe"))      _PuttyPath = programs+@"\putty\putty.exe";
            else _PuttyPath = null;
        }

        //--- User_PropertyChanged --------------------------------------
        private void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            bool enabled = RxGlobals.User.UserType >= EUserType.usr_service;
            NetworkGrid.Columns["PuttyDebug"].Visibility=(_PuttyPath!=null && enabled) ? Visibility.Visible : Visibility.Collapsed; 
            NetworkGrid.Columns["Putty"     ].Visibility=(_PuttyPath!=null && enabled) ? Visibility.Visible : Visibility.Collapsed;
            Button_Delete.IsEnabled = enabled;
            Button_Save.IsEnabled   = enabled;
            RxGlobals.Network.IsEnabled=enabled;
            NetworkGrid.InvalidateData();
        }

        //--- NetworkGrid_CellExitedEditMode -------------------------------------------------------
        private void NetworkGrid_CellExitedEditMode(object sender, Infragistics.Controls.Grids.CellExitedEditingEventArgs e)
        {
            if (NetworkGrid.ActiveItem!=null)
                (NetworkGrid.ActiveItem as NetworkItem).SendMsg(TcpIp.CMD_SET_NETWORK);
        }
        
        //--- ComboBox_GotFocus ----------------------------------------------
        private ComboBox _ActItem;
        private int      _ActValue;
        private int      _ActSel;
        private void ComboBox_GotFocus(object sender, RoutedEventArgs e)
        {
            _ActItem  = sender as ComboBox;
            _ActSel   = _ActItem.SelectedIndex;
            try {_ActValue = Convert.ToInt32(_ActItem.Text);}
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                _ActValue=0;
            }
        }

        //--- ComboBox_LostFocus ------------------------------------------------
        private void ComboBox_LostFocus(object sender, RoutedEventArgs e)
        {
            _ActItem  = null;
            _ActSel   = 0;
            _ActValue = 0;
        }
         
        //--- DeviceNo_Changed ----------------------------------------------
        private void DeviceNo_Changed(object sender, SelectionChangedEventArgs e)
        {
            // do not allow changing to 0!! (Spooler)
            if (NetworkGrid.ActiveItem!=null)
            {
                NetworkItem item = NetworkGrid.ActiveItem as NetworkItem;
                if (item.DeviceNo==0 && _ActValue!=0)
                {
                    _ActItem.SelectedIndex = _ActSel;
                }
                else item.SendMsg(TcpIp.CMD_SET_NETWORK);
            }
        }

        //--- Flash_Clicked ------------------------------------------------------------
        private void Flash_Clicked(object sender, RoutedEventArgs e)
        {
            Button button = sender as Button;
            if (button != null)
            {
                NetworkItem item = button.DataContext as NetworkItem;
                if (item != null)
                {
                    item.Flash = 1 - item.Flash;
                    item.SendMsg(TcpIp.CMD_SET_NETWORK);
                }
            }
        }

        //--- Keyboard_Clicked ------------------------------------------------------------
        private void Keyboard_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.Keyboard.Visibility = Visibility.Visible;
            RxGlobals.TaskSwitch.Visibility = Visibility.Visible;
        }

        //--- PuttyDebug_Clicked ------------------------------------------------------------
        private void PuttyDebug_Clicked(object sender, RoutedEventArgs e)
        {
            Button button = sender as Button;
            if (button != null)
            {
                NetworkItem item = button.DataContext as NetworkItem;
                if (item != null)
                {
                    string par = string.Format("-i c:\\gui\\id_rsa.ppk -P 777 -raw root@{0} ", item.IpAddr);
                    Rx.StartProcess(_PuttyPath, par);
                    //    RxGlobals.Keyboard.Visibility = Visibility.Visible;
                }
            }
        }

        //--- Putty_Clicked ------------------------------------------------------------
        private void Putty_Clicked(object sender, RoutedEventArgs e)
        {
            Button button = sender as Button;
            if (button != null)
            {
                NetworkItem item = button.DataContext as NetworkItem;
                if (item != null)
                {
                    string par = string.Format("-i c:\\gui\\id_rsa.ppk -ssh root@{0}", item.IpAddr);
                    Rx.StartProcess(_PuttyPath, par);
                    //     RxGlobals.Keyboard.Visibility = Visibility.Visible;
                }
            }
        }

        //--- Save_Clicked ------------------------------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_NETWORK_SAVE);
        }

        //--- Reload_Clicked ----------------------------------------------------------
        private void Reload_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_NETWORK_RELOAD);
        }

        //--- Delete_Clicked ----------------------------------------------------------
        private void Delete_Clicked(object sender, RoutedEventArgs e)
        {
            if (NetworkGrid.ActiveItem!=null)
            {
                (NetworkGrid.ActiveItem as NetworkItem).SendMsg(TcpIp.CMD_NETWORK_DELETE);
            }
        }
    }
}
