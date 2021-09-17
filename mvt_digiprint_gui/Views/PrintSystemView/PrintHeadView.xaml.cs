using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
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

namespace RX_DigiPrint.Views.PrintSystemView
{
    public partial class PrintHeadView : UserControl
    {
        public PrintHeadView(int no)
        {
            No=no;
            // DataContext.type = HeadStat
            InitializeComponent();
            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
            _PrinterType_Changed();
            RxGlobals.User.PropertyChanged += User_PropertyChanged;
            RxGlobals.Chiller.PropertyChanged += Chiller_PropertyChanged;
            User_PropertyChanged(null, null);
            Chiller_PropertyChanged(null, null);

//          try{ FpVoltage.Text = RxGlobals.PrintSystem.HeadFpVoltage[_no].ToString();}
//          catch (Exception) { FpVoltage.Text = "";};
        }

        //--- UserControl_DataContextChanged -----------------------------------
        private void UserControl_DataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            HeadStat stat = DataContext as HeadStat;
            if (stat!= null) stat.PropertyChanged += stat_PropertyChanged;
        }

        //--- stat_PropertyChanged ----------------------------------------------
        void stat_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Name")) Chiller_PropertyChanged(this, null);
            if (e.PropertyName.Equals("HeadFpVoltage"))
            {
                try{ FpVoltage.Text = RxGlobals.PrintSystem.HeadFpVoltage[_No].ToString();}
                catch (Exception) { FpVoltage.Text = "";};                
            }
        }

        //--- ServiceGrid_IsVisibleChanged -----------------
        private void ServiceGrid_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool) e.NewValue)
            {
                if (RxGlobals.PrintSystem.HeadFpVoltage!=null)
                    FpVoltage.Text = RxGlobals.PrintSystem.HeadFpVoltage[_No].ToString();
            }
        }

        //--- Property No ---------------------------------------
        private int _No;
        public int No
        {
            get { return _No; }
            set { _No=value; }
        }
        
        //--- Chiller_PropertyChanged ----------------------------------
        private void Chiller_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            HeadStat stat = DataContext as HeadStat;
            if (stat==null || stat.Name==null || stat.Name.StartsWith("?"))
                CmdButton.IsEnabled = false;
            else CmdButton.IsEnabled = RxGlobals.Chiller.Running;
        }

        //--- User_PropertyChanged --------------------------------------
        private void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            Visibility visibility  =  (RxGlobals.User.UserType >= EUserType.usr_service) ? Visibility.Visible : Visibility.Collapsed; 
            ServiceGrid.Visibility = visibility;
        }

        //--- PrintSystem_PropertyChanged ---------------------------------------------------------
        private void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("PrinterType")) _PrinterType_Changed();
            if (e.PropertyName.Equals("HeadFpVoltage"))
            {
                try{ FpVoltage.Text = RxGlobals.PrintSystem.HeadFpVoltage[_No].ToString();}
                catch (Exception) { FpVoltage.Text = "";};                
            }
        }

        //--- _PrinterType_Changed ----------------------------------------------------
        private void _PrinterType_Changed()
        {
            Grid.RowDefinitions[1].Height = new GridLength(25/RxGlobals.Screen.Scale);

            //--- printer_test_slide_only ------------------------------------------------------------
            Visibility visible   = (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide_only) ? Visibility.Visible   : Visibility.Collapsed;
            Visibility collapsed = (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide_only) ? Visibility.Collapsed : Visibility.Visible;
            Button_PurgeHard.Visibility = collapsed;
            Button_PurgeSoft.Visibility = collapsed;
            Button_PurgeMicro.Visibility = collapsed;

            //--- printer_test_table_seon -----------------------------------------------------------
            visible   = (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_table_seon || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide 
                        || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide_HB) ? 
                        Visibility.Visible   : Visibility.Collapsed;
            collapsed = (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_table_seon || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide 
                        || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide_HB) ? 
                        Visibility.Collapsed : Visibility.Visible;
            Button_ToggleMeniscus.Visibility = visible;
            visible = (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_table_seon) ? Visibility.Visible : Visibility.Collapsed;
            collapsed = (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_table_seon) ? Visibility.Collapsed : Visibility.Visible;
            Cooler_Pres.Visibility = visible;
            Cooler_Temp.Visibility = visible;
            ClusterNo.Visibility = visible;
            Temp_Head.Visibility = Temp_Head_Warn.Visibility = collapsed;


            Button_Purge4Ever.Visibility = (RxGlobals.User.UserType >= EUserType.usr_mouvent) ? Visibility.Visible : Visibility.Collapsed;
        }
        
        //--- FpVoltage_TextChanged -----------------------------------------
        private void FpVoltage_TextChanged(object sender, TextChangedEventArgs e)
        {
            RxGlobals.PrintSystem.SetFpVoltage(_No, Rx.StrToInt32(FpVoltage.Text));
        }

        //--- Cmd_Clicked ---------------------------
        private void Cmd_Clicked(object sender, RoutedEventArgs e)
        {
            HeadStat stat = DataContext as HeadStat;
            if (stat == null) return;

            if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide_only || RxGlobals.User.UserType>=EUserType.usr_service)
                Button_Print.Visibility  = Button_Off.Visibility = CommandLine1.Visibility = Visibility.Visible;
            else Button_Print.Visibility = Button_Off.Visibility = CommandLine1.Visibility = Visibility.Collapsed;

            Visibility visible = RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_table_seon ? Visibility.Visible : Visibility.Collapsed;

            Button_PrintCluster.Visibility = visible;
            Button_PurgeCluster.Visibility = visible;

            CmdPopup.Open(CmdButton);
        }

        //--- _command --------------------
        private void _command(string name, EFluidCtrlMode cmd)
        {
            HeadStat stat = DataContext as HeadStat;
            if (stat!=null)
            {
                //--- do this also for other printers? --------------
                if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf)
                {
                    if (name!=null && !RxGlobals.StepperStatus[0].X_in_cap) // when slide not under printhead
                    {
                        if (!MvtMessageBox.YesNo(name, "Printhead NOT in capping position. Continue?",  MessageBoxImage.Question, true)) return;
                    }
                }

                TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
                msg.no       = stat.HeadNo;
                msg.ctrlMode = cmd;

                RxGlobals.RxInterface.SendMsg(TcpIp.CMD_HEAD_FLUID_CTRL_MODE, ref msg);
            }
            CmdPopup.IsOpen = false;
        }

        /*
        private bool _RobotUsed()
        {
            HeadStat stat = DataContext as HeadStat;
            switch (RxGlobals.PrintSystem.PrinterType)
            {
                case EPrinterType.printer_LB702_WB:
                    if (RxGlobals.PrintSystem.ColorCnt % 2 == 0 && stat != null)
                    {
                        if ((stat.InkSupply.No - 1) / 2 < RxGlobals.StepperStatus.Length)
                            return RxGlobals.StepperStatus[(stat.InkSupply.No - 1) / 2].ClnUsed;
                        else
                            return false;
                    }
                    break;

                default: return false;
            }
            return false;
        }
        */

        //--- Commands ------------------------------------
        private void OFF_Clicked        (object sender, RoutedEventArgs e) {_command(null, EFluidCtrlMode.ctrl_off);          }
        private void Print_Clicked      (object sender, RoutedEventArgs e) {_command(null, EFluidCtrlMode.ctrl_prepareToPrint);        }
        private void PurgeSoft_Clicked  (object sender, RoutedEventArgs e) {_command("Purge", EFluidCtrlMode.ctrl_purge_soft);   }
        private void PurgeMicro_Clicked (object sender, RoutedEventArgs e) {_command("Purge", EFluidCtrlMode.ctrl_purge_soft);  }
        private void Purge_Clicked      (object sender, RoutedEventArgs e)
        {
            if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide_only)
                _command("Purge", EFluidCtrlMode.ctrl_purge_step2);
            else
                _command("Purge", EFluidCtrlMode.ctrl_purge);
        }
        private void PurgeHard_Clicked  (object sender, RoutedEventArgs e) {_command("Purge", EFluidCtrlMode.ctrl_purge_hard);   }

        private void Purge4Ever_Clicked(object sender, RoutedEventArgs e) { _command("Purge", EFluidCtrlMode.ctrl_purge4ever);   }
        //       private void Drain_Clicked      (object sender, RoutedEventArgs e) {_command(EFluidCtrlMode.ctrl_drain);        }
        private void Flush_Clicked      (object sender, RoutedEventArgs e) {_command("Flush", EFluidCtrlMode.ctrl_flush_night);   }
        private void Wipe_Clicked       (object sender, RoutedEventArgs e) {_command(null, EFluidCtrlMode.ctrl_wipe);             }
        private void ToggleMeniscus_Clicked(object sender, RoutedEventArgs e){_command(null, EFluidCtrlMode.ctrl_toggle_meniscus);}

        private void PrintCluster_Clicked(object sender, RoutedEventArgs e)
        {
            HeadStat stat = DataContext as HeadStat;
            if (stat != null)
            {
                int cluster = stat.HeadNo/4;
                TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();

                for (int i = 0; i < 4; i++)
                {
                    msg.no = cluster * 4 + i;
                    msg.ctrlMode = EFluidCtrlMode.ctrl_prepareToPrint;
                    RxGlobals.RxInterface.SendMsg(TcpIp.CMD_HEAD_FLUID_CTRL_MODE, ref msg);
                }
                CmdPopup.IsOpen = false;
            }
        }

        private void PurgeCluster_Clicked(object sender, RoutedEventArgs e)
        {
            HeadStat stat = DataContext as HeadStat;
            if (stat != null)
            {
                TcpIp.SValue msg = new TcpIp.SValue();
                msg.no = stat.HeadNo;
                    
                RxGlobals.RxInterface.SendMsg(TcpIp.CMD_PURGE_CLUSTER, ref msg);

                CmdPopup.IsOpen = false;
            }
        }
    }
}
