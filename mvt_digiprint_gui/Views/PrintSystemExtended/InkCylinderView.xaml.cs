using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.PrintSystemExtendedView
{
    /// <summary>
    /// Interaction logic for InkCylinderView.xaml
    /// </summary>
    public partial class InkCylinderView : UserControl, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        private InkSupply _InkSupply;

        private string _header { get; set; }
        public string Header
        {
            get { return _header; }
            set { _header = value; OnPropertyChanged("Header"); }
        }

        public InkCylinderView()
        {
            InitializeComponent();

            RxGlobals.Chiller.PropertyChanged += Chiller_PropertyChanged;

        }
       
        private void Chiller_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            CmdButton.IsEnabled = RxGlobals.Chiller.Running && _InkSupply != null && _InkSupply.InkType != null;
        }

        public void SetUser()
        {
            if (RxGlobals.User.UserType >= Services.EUserType.usr_service)
            {
                ServiceGrid.Visibility = System.Windows.Visibility.Visible;
            }
            else
            {
                ServiceGrid.Visibility = System.Windows.Visibility.Hidden;
            }

            Visibility visibility = (RxGlobals.User.UserType >= EUserType.usr_service) ? Visibility.Visible : Visibility.Collapsed;
            Line_Calibrate.Visibility = visibility;
            // Button_Calibrate.Visibility = visibility;
            InkTypeCombo.IsEnabled = (RxGlobals.User.UserType >= EUserType.usr_service);
        }

        private void InkCylinderViewDataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            // old data context:
            if (_InkSupply != null)
            {
                _InkSupply.PropertyChanged -= OnInkSupplyPropertyChanged;
            }

                // new data context:
            _InkSupply = DataContext as InkSupply;
            _InkSupply.PropertyChanged += OnInkSupplyPropertyChanged;
            InkTypeCombo.DataContext = _InkSupply;
        }

        private void UserControl_Loaded(object sender, RoutedEventArgs e)
        {
            _InkSupply = DataContext as InkSupply;
        }

        private void UserControl_Unloaded(object sender, RoutedEventArgs e)
        {
            _InkSupply.PropertyChanged -= OnInkSupplyPropertyChanged;

            // RxGlobals.PrintSystem.PropertyChanged -= PrintSystem_PropertyChanged;
        }

        private void OnInkSupplyPropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (_InkSupply != null && e.PropertyName.Equals("CtrlMode"))
            {
                if (_InkSupply.CtrlMode == EFluidCtrlMode.ctrl_fill)
                {
                    MsgText.Text = "Put new ink canister.";
                    MsgPopup.IsOpen = true;
                }
                else if (_InkSupply.CtrlMode == EFluidCtrlMode.ctrl_empty_step1)
                {
                    MsgText.Text = "Remove fill tube.\n"
                               + "\n"
                               + "Put bleed tube to empty canister.";
                    MsgPopup.IsOpen = true;
                }
                else MsgPopup.IsOpen = false;
            }

            if (e.PropertyName.Equals("InkType"))
            {
                FlushButton.DataContext = _InkSupply.InkType;
                Chiller_PropertyChanged(this, null);
                RxGlobals.PrintSystem.Changed = true;
            }
        }

        /*
        private bool _intialized = false;
        private void ColorCombo_DropDownOpening(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (_intialized) return;

            XamMultiColumnComboEditor editor = sender as XamMultiColumnComboEditor;
            if (VisualTreeHelper.GetChildrenCount(editor) == 0) return;

            _intialized = true;
        }
        */

        private void InkSupplyCommands_Clicked(object sender, RoutedEventArgs e)
        {
            if (CmdPopup.IsOpen)
            {
                CmdPopup.IsOpen = false;
            }
            else
            {
                CmdPopup.Open(CmdButton);
            }
        }

        private void Flush_Clicked(object sender, RoutedEventArgs e)
        {
            FlushPopup.Open(FlushButton);
        }

        private void _command(string name, EFluidCtrlMode cmd, bool all)
        {
            InkSupply supply = DataContext as InkSupply;
            if (supply != null)
            {
                //--- do this also for other printers? --------------
                if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_cleaf)
                {
                    if (name != null) // && !RxGlobals.TestTableStatus.Z_in_cap)
                    {
                        if (!MvtMessageBox.YesNo(name, "Insert purge tray. Continue?", MessageBoxImage.Question, true)) return;
                    }
                }

                TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
                if (all) msg.no = -1;
                else msg.no = supply.No - 1;
                msg.ctrlMode = cmd;

                RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
            }
            CmdPopup.IsOpen = false;
            FlushPopup.IsOpen = false;
            MsgPopup.IsOpen = false;
        }

        private void OnOff_Clicked(object sender, RoutedEventArgs e)
        {
            if (_InkSupply.CtrlMode == EFluidCtrlMode.ctrl_print)
                _command(null, EFluidCtrlMode.ctrl_shutdown, false);
            else
                _command(null, EFluidCtrlMode.ctrl_off, false);
        }

        private void ShutDown_Clicked(object sender, RoutedEventArgs e) { _command(null, EFluidCtrlMode.ctrl_off, false); }
        private void Check_Clicked(object sender, RoutedEventArgs e) { _command(null, EFluidCtrlMode.ctrl_check_step0, false); }
        private void Calibrate_Clicked(object sender, RoutedEventArgs e) { _command(null, EFluidCtrlMode.ctrl_cal_start, false); }
        private void Print_Clicked(object sender, RoutedEventArgs e) { _command(null, EFluidCtrlMode.ctrl_print, false); }
        private void Fill_Clicked(object sender, RoutedEventArgs e) { _command(null, EFluidCtrlMode.ctrl_fill, false); }
        private void Empty_Clicked(object sender, RoutedEventArgs e) { _command("Empty", EFluidCtrlMode.ctrl_empty, false); }
        private void Flush_Clicked_0(object sender, RoutedEventArgs e) { _command("Flush", EFluidCtrlMode.ctrl_flush_night, false); }
        private void Flush_Clicked_1(object sender, RoutedEventArgs e) { _command("Flush", EFluidCtrlMode.ctrl_flush_weekend, false); }
        private void Flush_Clicked_2(object sender, RoutedEventArgs e) { _command("Flush", EFluidCtrlMode.ctrl_flush_week, false); }


        private void Pressure_LostFocus(object sender, RoutedEventArgs e)
        {
            double pressure = (sender as MvtNumBox).Value * 10;
            if (pressure != _InkSupply.CylinderPresSet)
            {
                TcpIp.SValue msg = new TcpIp.SValue() { no = _InkSupply.No - 1, value = (Int32)pressure };
                RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_PRESSURE, ref msg);
                RxGlobals.PrintSystem.Changed = true;
            }
        }

        private void InkTypeCombo_LostFocus(object sender,RoutedEventArgs e)
        {
            int cnt=RxGlobals.PrintSystem.InkCylindersPerColor;
            if (cnt>1) // set the same for all ink supplies
            {
                int no=(_InkSupply.No-1) / cnt;
                for (int i=0; i<cnt; i++)
                {
                    RxGlobals.InkSupply.List[no+i].InkType = _InkSupply.InkType;
                }
            }
        }

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
            if (MvtMessageBox.YesNo("Purge + Wipe", "PURGE and WIPE all printheads?", MessageBoxImage.Question, true))
            {
                _command("Purge+Wipe", EFluidCtrlMode.ctrl_purge_hard_wipe, true);
            }
        }

        private void PurgeWash_Clicked(object sender, RoutedEventArgs e)
        {
            if (MvtMessageBox.YesNo("Purge + Wash", "PURGE and WASH all printheads?", MessageBoxImage.Question, true))
            {
                _command("Purge+Wash", EFluidCtrlMode.ctrl_purge_hard_wash, true);
            }
        }

        private void Purge4Ever_Clicked(object sender, RoutedEventArgs e)
        {
            if (MvtMessageBox.YesNo("Purge + Wash", "PURGE and WASH all printheads?", MessageBoxImage.Question, true))
            {
                _command("Purge+Wash", EFluidCtrlMode.ctrl_purge_hard_wash, true);
            }
        }

        private void Done_Clicked(object sender, RoutedEventArgs e)
        {
            _command(null, _InkSupply.CtrlMode + 1, false);
        }
    }
}
