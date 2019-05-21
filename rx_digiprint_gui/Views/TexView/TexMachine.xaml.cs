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

namespace RX_DigiPrint.Views.TexView
{
    /// <summary>
    /// Interaction logic for TexMachine.xaml
    /// </summary>
    public partial class TexMachine : UserControl
    {
        public TexMachine()
        {
            InitializeComponent();
            RxGlobals.Timer.TimerFct += _Tick;
            RxGlobals.Plc.PropertyChanged += Plc_PropertyChanged;
            RxGlobals.PrinterStatus.PropertyChanged += PrinterStatusChanged;
        }

        //--- _Tick --------------------------
        private void _Tick(int no)
        {
            if (RxGlobals.Plc.Connected)
            {
                StringBuilder str = new StringBuilder(2048);
                int i;

                str.AppendFormat("{0}\n", RxGlobals.Plc.UnitID);
                for (i=0; i<ButtonsGrid.Children.Count; i++)
                {
                    RxButton button = ButtonsGrid.Children[i] as RxButton;
                    if (button!=null) str.AppendFormat("{0}\n", button.Name);
                }

                RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_GET_VAR, str.ToString());
            }
        }

        //--- Plc_PropertyChanged --------------------------------
        void Plc_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            int i;
            for (i=0; i<ButtonsGrid.Children.Count; i++)
            {
                RxButton button = ButtonsGrid.Children[i] as RxButton;
                if (button!=null)
                {
                    string str = RxGlobals.Plc.GetVar(RxGlobals.Plc.UnitID, button.Name);
                    bool check = false;
                    if (str!=null && str.ToLower().Equals("true")) check=true;
                    if (button.Name.Equals("PAR_DRYER_ON"))
                    {
                        button.IsChecked = true;
                        button.IsBusy    = check;
                    }
                    else button.IsChecked = check;
                }
            }
        }


        //--- PrinterStatusChanged ----------------------------------------
        private void PrinterStatusChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            bool enabled = (RxGlobals.PrinterStatus.PrintState!=EPrintState.ps_printing) && (RxGlobals.PrinterStatus.PrintState!=EPrintState.ps_stopping);
            for (int i=0; i<ButtonsGrid.Children.Count; i++)
            {
                RxButton button = ButtonsGrid.Children[i] as RxButton;
                if (button!=null)
                {
                    if (button.Name.Equals("PAR_ALL_ON")) 
                        button.IsEnabled = (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_off) 
                                        || (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_pause)
                                        || (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_ready_power);
                    else 
                        button.IsEnabled = enabled;
                }
            }
        }

        //--- OnOff_Clicked --------------------------------------------
        private void OnOff_Clicked(object sender, RoutedEventArgs e)
        {
            RxButton button = sender as RxButton;
            if (button!=null && button.IsEnabled) 
            {
                bool check = button.IsChecked;
                if (button.Name.Equals("PAR_DRYER_ON")) check = button.IsBusy;
                else                                    check = button.IsChecked;

                RxGlobals.Plc.SetVar(button.Name, !check);
            }
        }
    }
}
