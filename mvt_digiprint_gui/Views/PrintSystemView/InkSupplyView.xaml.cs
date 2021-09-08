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
            User_PropertyChanged(null, null);
        }

        //--- Chiller_PropertyChanged ----------------------------------
        private void Chiller_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            CmdButton.IsEnabled = RxGlobals.Chiller.Running && _InkSupply!=null && _InkSupply.InkType!=null;
        }

        //--- User_PropertyChanged --------------------------------------
        private void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            Visibility visibility  = (RxGlobals.User.UserType >= EUserType.usr_service) ? Visibility.Visible : Visibility.Collapsed; 
            ServiceGrid.Visibility = visibility;
            Line_Calibrate.Visibility = visibility;
            // Button_Calibrate.Visibility = visibility;
            InkType.IsEnabled       =  (RxGlobals.User.UserType >= EUserType.usr_service);
            CB_RectoVerso.IsEnabled =  (RxGlobals.User.UserType >= EUserType.usr_service);

            visibility = ((RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide_HB || RxGlobals.PrintSystem.IsLb)
                && RxGlobals.User.UserType >= EUserType.usr_service) ? Visibility.Visible : Visibility.Collapsed;
            Button_Recovery.Visibility = visibility;
        }

        //--- UserControl_Loaded -----------------------------------------------
        private void UserControl_Loaded(object sender, RoutedEventArgs e)
        {
            _InkSupply = DataContext as InkSupply;
             
            InkType.DataContext = _InkSupply;
            _InkSupply.PropertyChanged += OnInkSupplyPropertyChanged;
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
            if  (e.PropertyName.Equals("ColorCnt") || e.PropertyName.Equals("HeadsPerColor") 
                || e.PropertyName.Equals("InkCylindersPerColor"))
            {
                _set_checkbox_style();
            }
        }

        //--- PrinterType_Changed -----------------------------------------
        public void PrinterType_Changed()
        {
            int i;
            Visibility visible;
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
                    NotConnected.Foreground = Rx.BrushError;
                    break;
            }
            if (RxGlobals.PrintSystem.HasHeater) RowHeight_Heater.Height = new GridLength(1, GridUnitType.Auto);
            else                                 RowHeight_Heater.Height = new GridLength(0);

            Button_PurgeVacc.Visibility = (RxGlobals.PrintSystem.IsTx || RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_LB702_WB) ? Visibility.Visible : Visibility.Collapsed;
            Button_PurgeWipe.Visibility = (RxGlobals.PrintSystem.IsTx) ? Visibility.Visible : Visibility.Collapsed;
            visible = RxGlobals.StepperStatus[0].RobotUsed ? Visibility.Visible : Visibility.Collapsed;
            for (i = 0; i < RxGlobals.StepperStatus.Length; i++)
            {
                if (RxGlobals.StepperStatus[i].RobotUsed) visible = Visibility.Visible;
            }
            Button_PurgeWash.Visibility = visible;

            visible = ((RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide || RxGlobals.PrintSystem.PrinterType== EPrinterType.printer_test_slide_HB || RxGlobals.PrintSystem.IsLb) 
                && RxGlobals.User.UserType >= EUserType.usr_service) ? Visibility.Visible : Visibility.Collapsed;
            Button_Recovery.Visibility = visible;
        }

        //--- GetRowDefinitions ----------------------
        public RowDefinitionCollection GetRowDefinitions()
		{
            return  SettingsGrid.RowDefinitions;
		}

        //--- OnInkSupplyPropertyChanged -------------------------
        private void OnInkSupplyPropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (_InkSupply!=null && e.PropertyName.Equals("CtrlMode"))
            {
                if (_InkSupply.CtrlMode==EFluidCtrlMode.ctrl_fill_step1)
                {
                    MsgText.Text="Put new ink canister.";
                    MsgPopup.IsOpen=true;
                }
                else if (_InkSupply.CtrlMode==EFluidCtrlMode.ctrl_empty_step1)
                {
                    MsgText.Text="Remove fill tube.\n"
                               + "\n"
                               +  "Put bleed tube to empty canister.";
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
                        if (!MvtMessageBox.YesNo(name, "Insert purge tray. Continue?",  MessageBoxImage.Question, true)) return;
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
        private void Print_Clicked      (object sender, RoutedEventArgs e) {_command(null,      EFluidCtrlMode.ctrl_prepareToPrint, false);}
        private void Fill_Clicked       (object sender, RoutedEventArgs e) {_command(null,      EFluidCtrlMode.ctrl_fill,           false);}
        private void Empty_Clicked      (object sender, RoutedEventArgs e) {_command("Empty",   EFluidCtrlMode.ctrl_empty,          false);}
        private void Flush_Clicked_0    (object sender, RoutedEventArgs e) {_command("Flush",   EFluidCtrlMode.ctrl_flush_night,    false);}
        private void Flush_Clicked_1    (object sender, RoutedEventArgs e) {_command("Flush",   EFluidCtrlMode.ctrl_flush_weekend,  false);}
        private void Flush_Clicked_2    (object sender, RoutedEventArgs e) {_command("Flush",   EFluidCtrlMode.ctrl_flush_week,     false);}

        private void Purge_Clicked(object sender, RoutedEventArgs e)
        {
            RX_Common.MvtMessageBox.EPurgeResult result = MvtMessageBox.Purge("Purge", "Purge " + _InkSupply.InkType.Name + " ?");
            if (result == MvtMessageBox.EPurgeResult.PurgeResultYes || result == MvtMessageBox.EPurgeResult.PurgeResultAll)
            {
                _command("Purge", EFluidCtrlMode.ctrl_purge_hard, (result == MvtMessageBox.EPurgeResult.PurgeResultAll)); 
            }
        }

        private void PurgeVacc_Clicked(object sender, RoutedEventArgs e)
        {
            RX_Common.MvtMessageBox.EPurgeResult result = MvtMessageBox.Purge("Purge", "Purge and Vacuum " + _InkSupply.InkType.Name + " ?");
            if (result == MvtMessageBox.EPurgeResult.PurgeResultYes || result == MvtMessageBox.EPurgeResult.PurgeResultAll)
            {
                _command("Purge+Vacc", EFluidCtrlMode.ctrl_purge_hard_vacc, (result == MvtMessageBox.EPurgeResult.PurgeResultAll));
            }
        }
        private void PurgeWipe_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxGlobals.PrintSystem.IsTx)
            {
                if (MvtMessageBox.YesNo("Purge + Wipe", "PURGE and WIPE all printheads?", MessageBoxImage.Question, true))
                {
                    _command("Purge+Wipe", EFluidCtrlMode.ctrl_purge_hard_wipe, true);
                }
            }
            else
            {
                RX_Common.MvtMessageBox.EPurgeResult result =
                MvtMessageBox.Purge("Purge", "Purge and Wipe " + _InkSupply.InkType.Name + " ?");
                if (result == MvtMessageBox.EPurgeResult.PurgeResultYes || result == MvtMessageBox.EPurgeResult.PurgeResultAll)
                {
                    _command("Purge+Wipe", EFluidCtrlMode.ctrl_purge_hard_wipe, (result == MvtMessageBox.EPurgeResult.PurgeResultAll));
                }
            }
        }

        private void Purge4Ever_Clicked(object sender, RoutedEventArgs e)
        {
            RX_Common.MvtMessageBox.EPurgeResult result = MvtMessageBox.Purge("Purge", "Purge for ever " + _InkSupply.InkType.Name + " ?");
            if (result == MvtMessageBox.EPurgeResult.PurgeResultYes || result == MvtMessageBox.EPurgeResult.PurgeResultAll)
            {
                _command("Purge4Ever", EFluidCtrlMode.ctrl_purge4ever, (result == MvtMessageBox.EPurgeResult.PurgeResultAll));
            }
        }

        private void Recovery_Clicked(object sender, RoutedEventArgs e)
        {
            RX_Common.MvtMessageBox.EPurgeResult result = MvtMessageBox.Purge("Recover", "Recover " + _InkSupply.InkType.Name + "?");
            if (result == MvtMessageBox.EPurgeResult.PurgeResultYes || result == MvtMessageBox.EPurgeResult.PurgeResultAll)
            {
                _command("Recovery", EFluidCtrlMode.ctrl_recovery_start, (result == MvtMessageBox.EPurgeResult.PurgeResultAll));
            }
        }

        private void Wipe_Clicked(object sender, RoutedEventArgs e)
        {
            RX_Common.MvtMessageBox.EPurgeResult result = MvtMessageBox.Purge("Wipe", "Wipe " + _InkSupply.InkType.Name + " ?");
            if (result == MvtMessageBox.EPurgeResult.PurgeResultYes || result == MvtMessageBox.EPurgeResult.PurgeResultAll)
            {
                _command("Wipe", EFluidCtrlMode.ctrl_wipe, (result == MvtMessageBox.EPurgeResult.PurgeResultAll));
            }
        }

        private void PurgeWash_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxGlobals.StepperStatus[0].RobotUsed)
            {
                RX_Common.MvtMessageBox.EPurgeResult result = MvtMessageBox.Purge("Purge + Wash", "Purge + Wash " + _InkSupply.InkType.Name + " ?");
                if (result == MvtMessageBox.EPurgeResult.PurgeResultYes || result == MvtMessageBox.EPurgeResult.PurgeResultAll)
                {
                    _command("Purge+Wash", EFluidCtrlMode.ctrl_purge_hard_wash, (result == MvtMessageBox.EPurgeResult.PurgeResultAll));
                }
            }
            else
            {
                if (MvtMessageBox.YesNo("Purge + Wash", "PURGE and WASH all printheads?", MessageBoxImage.Question, true))
                {
                    _command("Purge+Wash", EFluidCtrlMode.ctrl_purge_hard_wash, true);
                }
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

        public Action LayoutUpdated = null;
		private void SettingsGrid_LayoutUpdated(object sender,EventArgs e)
		{
            if (LayoutUpdated!=null) LayoutUpdated();
		}
	}
}
