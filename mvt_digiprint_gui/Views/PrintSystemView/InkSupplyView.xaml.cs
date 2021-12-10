using Infragistics.Controls.Editors;
using Infragistics.Controls.Editors.Primitives;
using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Media;

namespace RX_DigiPrint.Views.PrintSystemView
{
    public partial class InkSupplyView : UserControl, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        private void OnPropertyChanged(string name)
        {
            if(PropertyChanged != null)
            {
                PropertyChanged.Invoke(this, new PropertyChangedEventArgs(name));
            }
        }

        private bool _HasSideSelection;
        public bool HasSideSelection
        {
            get { return _HasSideSelection; }
            set { _HasSideSelection = value; OnPropertyChanged("HasSideSelection"); }
        }


        private InkSupply   _InkSupply;
        private static int  _ActiveItem=0;

        public InkSupplyView()
        {
            InitializeComponent();

            CB_RectoVerso.ItemsSource     = new EN_RectoVerso();
            TX_LungPressure.DataContext = RxGlobals.InkSupply.List[0];

            RxGlobals.User.PropertyChanged += User_PropertyChanged;
            RxGlobals.Chiller.PropertyChanged += Chiller_PropertyChanged;
            RxGlobals.InkSupply.List[Constants.Waste].PropertyChanged += InkSupply_PropertyChanged;
            User_PropertyChanged(null, null);
            InkSupply_PropertyChanged(null, null);
        }

        //--- Chiller_PropertyChanged ----------------------------------
        private void Chiller_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            CmdButton.IsEnabled = RxGlobals.Chiller.Running && _InkSupply!=null && _InkSupply.InkType!=null;
        }

        //--- User_PropertyChanged --------------------------------------
        private void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            Visibility visibility  = (RxGlobals.User.UserType >= EUserType.usr_maintenance) ? Visibility.Visible : Visibility.Collapsed; 
            ServiceGrid.Visibility = visibility;
            Line_Calibrate.Visibility = visibility;
            // Button_Calibrate.Visibility = visibility;
            InkType.IsEnabled       =  (RxGlobals.User.UserType >= EUserType.usr_maintenance);
            CB_RectoVerso.IsEnabled =  (RxGlobals.User.UserType >= EUserType.usr_maintenance);
        }

        //--- InkSupply_PropertyChanged ----------------------------------
        private void InkSupply_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            PurgeButton.IsEnabled = RxGlobals.InkSupply.List[Constants.Waste].CanisterLevel / 1000 < Constants.WasteMax;
        }

        //--- UserControl_Loaded -----------------------------------------------
        private void UserControl_Loaded(object sender, RoutedEventArgs e)
        {
            _InkSupply = DataContext as InkSupply;
            
            if (_InkSupply != null) { 
                InkType.DataContext = _InkSupply;
                _InkSupply.PropertyChanged += OnInkSupplyPropertyChanged;
            }
            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
            _set_checkbox_style();
            Chiller_PropertyChanged(this, null);
        }

        //--- UserControl_Unloaded ------------------------------------------------------
        private void UserControl_Unloaded(object sender, RoutedEventArgs e)
        {
            _InkSupply.PropertyChanged -= OnInkSupplyPropertyChanged;
            RxGlobals.PrintSystem.PropertyChanged -= PrintSystem_PropertyChanged;
        }

        //--- _set_checkbox_style ------------------------------
        private void _set_checkbox_style()
        {
            RxGlobals.PrintSystem.AllInkSupplies = (RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.HeadsPerColor) <= Constants.PrintSystem_MaxPrintHeads;
            CheckBox.Style = (Style)this.Resources[RxGlobals.PrintSystem.AllInkSupplies ? "CheckBoxStyle_OFF":"CheckBoxStyle_ON"];
            if (_ActiveItem<RxGlobals.InkSupply.List.Count)
            {
                RxGlobals.InkSupply.List[_ActiveItem].IsChecked = !RxGlobals.PrintSystem.AllInkSupplies;
                if (RxGlobals.PrintSystem.AllInkSupplies) _ActiveItem=0;
            }
        }

        //--- PrintSystem_PropertyChanged ----------------------------------
        void PrintSystem_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (_InkSupply == null)
            {
                _InkSupply = DataContext as InkSupply;
                CmdButton.IsEnabled = RxGlobals.Chiller.Running && _InkSupply != null && _InkSupply.InkType != null;
            }

            if  (e.PropertyName.Equals("ColorCnt") || e.PropertyName.Equals("HeadsPerColor") 
                || e.PropertyName.Equals("InkCylindersPerColor"))
            {
                _set_checkbox_style();
            }
            if (e.PropertyName.Equals("PrinterType"))
            {
                _printertype_changed();
            }
        }

        //--- _printertype_changed -----------------------------------------
        private void _printertype_changed()
        {
            int i = 0;
            CB_RectoVerso.Visibility = RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_DP803 ? Visibility.Visible : Visibility.Collapsed;
            HasSideSelection = RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_DP803;
            
            SettingsGrid.RowDefinitions[3].Height = new GridLength(25/RxGlobals.Screen.Scale);
            
            switch (RxGlobals.PrintSystem.PrinterType)
            {
                case EPrinterType.printer_test_slide_only:
                            for (i = 5; i < SettingsGrid.RowDefinitions.Count; i++) SettingsGrid.RowDefinitions[i].Height = new GridLength(0);
                            NotConnected.Foreground = Brushes.Transparent;
                            break;
                default:
                    for (i = 5; i < SettingsGrid.RowDefinitions.Count; i++) SettingsGrid.RowDefinitions[i].Height = new GridLength(1, GridUnitType.Auto);
                    NotConnected.Foreground = Brushes.Red;
                    break;
            }
            if (RxGlobals.PrintSystem.HasHeater) SettingsGrid.RowDefinitions[6].Height = new GridLength(1, GridUnitType.Auto);
            else                                 SettingsGrid.RowDefinitions[6].Height = new GridLength(0);

            Button_PurgeVacc.Visibility = (RxGlobals.PrintSystem.IsTx) ? Visibility.Visible : Visibility.Collapsed;
            Button_PurgeWipe.Visibility = (RxGlobals.PrintSystem.IsTx) ? Visibility.Visible : Visibility.Collapsed;

        }

        //--- OnInkSupplyPropertyChanged -------------------------
        private void OnInkSupplyPropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (_InkSupply!=null && e.PropertyName.Equals("CtrlMode"))
            {
                if (_InkSupply.CtrlMode==EFluidCtrlMode.ctrl_fill_step1)
                {
                    MsgText.Text = RX_DigiPrint.Resources.Language.Resources.PutNewInkCanister;
                    MsgPopup.IsOpen=true;
                }
                else if (_InkSupply.CtrlMode==EFluidCtrlMode.ctrl_empty_step1)
                {
                    MsgText.Text = RX_DigiPrint.Resources.Language.Resources.RemoveFillTube
                               + "\n"
                               + RX_DigiPrint.Resources.Language.Resources.PutBleedTubeToEmptyCanister;
                    MsgPopup.IsOpen=true;
                }
                else MsgPopup.IsOpen=false;
            }

            if (e.PropertyName.Equals("InkType"))
            {
                FlushButton.DataContext = _InkSupply.InkType;
                Chiller_PropertyChanged(this, null);
            }

            /*
            if (e.PropertyName.Equals("Flushed"))
            {
                Console.WriteLine("InkSupply[{0}]Flushed changed", _InkSupply.No);
            }
            */
        }

        //--- to adjust when styla changes ---------------------------
        // private int _border_width  = 3;
        // private int _header_height = 26;
        //------------------------------------------------------------

        //----------------------------------------------------------------
        private bool _intialized=false;
        private void ColorCombo_DropDownOpening(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (_intialized) return;

            XamMultiColumnComboEditor editor = sender as XamMultiColumnComboEditor;
            if (VisualTreeHelper.GetChildrenCount(editor)==0) return;

            _intialized = true;            
        }

        //--- Cmd_Clicked ---------------------------
        private void Cmd_Clicked(object sender, RoutedEventArgs e)
        {
           CmdPopup.Open(CmdButton);
        }

        //--- Flush_Clicked -------------------------------------
        private void Flush_Clicked(object sender, RoutedEventArgs e) 
        {
            FlushPopup.Open(FlushButton);
        }

        //--- _command --------------------
        private void _command(string name, EFluidCtrlMode cmd, bool all)
        {
            InkSupply supply = DataContext as InkSupply;
            if (supply!=null)
            {
                //--- do this also for other printers? --------------
                if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf)
                {
                    if (name!=null) // && !RxGlobals.StepperStatus[0].Z_in_cap)
                    {
                        if (!MvtMessageBox.YesNo(name, RX_DigiPrint.Resources.Language.Resources.InsertPurgeTray,  MessageBoxImage.Question, true)) return;
                    }
                }

                TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
                if (all) msg.no = -1;
                else     msg.no = supply.No-1;
                msg.ctrlMode = cmd;

                RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
            }
            CmdPopup.IsOpen     = false;
            FlushPopup.IsOpen   = false;
            MsgPopup.IsOpen     = false;
        }

        //--- OnOff_Clicked ----------------------------------------------------------
        private void OnOff_Clicked (object sender, RoutedEventArgs e)
        {
            if (_InkSupply.CtrlMode == EFluidCtrlMode.ctrl_print)
                _command(null, EFluidCtrlMode.ctrl_shutdown, false);
            else
                _command(null, EFluidCtrlMode.ctrl_off, false);
        }

        private void ShutDown_Clicked   (object sender, RoutedEventArgs e) {_command(null,      EFluidCtrlMode.ctrl_off,            false);}
        private void Check_Clicked      (object sender, RoutedEventArgs e) {_command(null,      EFluidCtrlMode.ctrl_check_step0,    false);}
        private void Calibrate_Clicked  (object sender, RoutedEventArgs e) {_command(null,      EFluidCtrlMode.ctrl_cal_start,      false);}
        private void Print_Clicked      (object sender, RoutedEventArgs e) {_command(null,      EFluidCtrlMode.ctrl_print,          false);}
        private void Fill_Clicked       (object sender, RoutedEventArgs e) {_command(null,      EFluidCtrlMode.ctrl_fill,           false);}
        private void Empty_Clicked      (object sender, RoutedEventArgs e) {_command("Empty",   EFluidCtrlMode.ctrl_empty,          false);}
        private void Flush_Clicked_0    (object sender, RoutedEventArgs e) {_command("Flush",   EFluidCtrlMode.ctrl_flush_night,    false);}
        private void Flush_Clicked_1    (object sender, RoutedEventArgs e) {_command("Flush",   EFluidCtrlMode.ctrl_flush_weekend,  false);}
        private void Flush_Clicked_2    (object sender, RoutedEventArgs e) {_command("Flush",   EFluidCtrlMode.ctrl_flush_week,     false);}

        private void Purge_Clicked(object sender, RoutedEventArgs e)
        {
            RX_Common.MvtMessageBox.EPurgeResult result =
                MvtMessageBox.Purge(RX_DigiPrint.Resources.Language.Resources.Purge, PrintSystemView.allowPurgeAll(), RX_DigiPrint.Resources.Language.Resources.Purge + _InkSupply.InkType.Name + " ?");
            if (result == MvtMessageBox.EPurgeResult.PurgeResultYes 
                || result == MvtMessageBox.EPurgeResult.PurgeResultAll)
            {
                _command(RX_DigiPrint.Resources.Language.Resources.Purge, EFluidCtrlMode.ctrl_purge_hard, (result == MvtMessageBox.EPurgeResult.PurgeResultAll)); 
            }
        }

        private void PurgeVacc_Clicked(object sender, RoutedEventArgs e)
        {
            RX_Common.MvtMessageBox.EPurgeResult result =
               MvtMessageBox.Purge(RX_DigiPrint.Resources.Language.Resources.Purge, PrintSystemView.allowPurgeAll(), RX_DigiPrint.Resources.Language.Resources.PurgeAndVacuum + _InkSupply.InkType.Name + " ?");
            if (result == MvtMessageBox.EPurgeResult.PurgeResultYes
                || result == MvtMessageBox.EPurgeResult.PurgeResultAll)
            {
                _command(RX_DigiPrint.Resources.Language.Resources.ShortPurgeAndVacc, EFluidCtrlMode.ctrl_purge_hard_vacc, (result == MvtMessageBox.EPurgeResult.PurgeResultAll));
            }
        }

        private void PurgeWipe_Clicked(object sender, RoutedEventArgs e) 
        {
            if (MvtMessageBox.YesNo(RX_DigiPrint.Resources.Language.Resources.ShortPurgeAndWipe, RX_DigiPrint.Resources.Language.Resources.PurgeAndWipeAllPrintheads,  MessageBoxImage.Question, true))
            {
                _command("Purge+Wipe",   EFluidCtrlMode.ctrl_purge_hard_wipe, true);
            }
        }

        private void Done_Clicked(object sender, RoutedEventArgs e)
        {
            _command(null, _InkSupply.CtrlMode+1, false);
        }

        //--- Pressure_LostFocus ------------------------------------------------
        private void Pressure_LostFocus(object sender, RoutedEventArgs e)
        {
            double pressure = (sender as MvtNumBox).Value * 10;
            if (pressure!=_InkSupply.CylinderPresSet)
            {
                TcpIp.SValue msg = new TcpIp.SValue(){no=_InkSupply.No-1, value=(Int32)pressure};
                RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_PRESSURE, ref msg);
                RxGlobals.PrintSystem.Changed = true;
            }
        }

        //--- CheckBox_Clicked -----------------------------------------
        private void CheckBox_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxGlobals.PrintSystem.AllInkSupplies) _InkSupply.IsChecked = false;
            else
            {
                int i;
                for(i=0; i<RxGlobals.InkSupply.List.Count; i++)
                {
                    if (RxGlobals.InkSupply.List[i]==_InkSupply) _ActiveItem=i;
                    else RxGlobals.InkSupply.List[i].IsChecked = false;
                }
            }
        }


    }
}
