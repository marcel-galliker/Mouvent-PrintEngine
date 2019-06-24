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

namespace RX_DigiPrint.Views.LB701UVView
{
    /// <summary>
    /// Interaction logic for ML_DiePos.xaml
    /// </summary>
    public partial class LB701UV_DiePos : UserControl
    {
        private static string UnitID="Application.GUI_00_001_Main";
        private double _Register;
        private int _Speed;

        public LB701UV_DiePos()
        {
            InitializeComponent();
        }

        //--- _MoveDie ---------------------
        private void _MoveDie(double dist)
        {            
            _Register = _Register+dist;
            string str = string.Format("{0}\n{1}={2:n3}\n", UnitID, "PAR_REGISTER_SETPOINT", _Register);
            str =  str.Replace(',', '.');
            RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_VAR, str);
        }

        //--- LeftLeft_Click -------------------------------
        private void LeftLeft_Clicked(object sender, RoutedEventArgs e)
        {
            _MoveDie(-1.0);
        }

        //--- Left_Click -------------------------------
        private void Left_Clicked(object sender, RoutedEventArgs e)
        {
            _MoveDie(-0.1);
        }

        //--- RightRight_Clicked -------------------------------
        private void RightRight_Clicked(object sender, RoutedEventArgs e)
        {
            _MoveDie(1.0);
        }

        //--- Right_Clicked -------------------------------
        private void Right_Clicked(object sender, RoutedEventArgs e)
        {
            _MoveDie(0.1);
        }

        //--- UserControl_IsVisibleChanged -----------------------
        private void UserControl_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue)
            {
                _Register = Rx.StrToDouble(RxGlobals.Plc.GetVar(UnitID, "PAR_REGISTER_SETPOINT"));
                _Speed = Rx.StrToInt32(RxGlobals.Plc.GetVar(UnitID, "PAR_PRINTING_SPEED"));
                RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_VAR, string.Format("{0}\n{1}={2}\n", UnitID, "PAR_PRINTING_SPEED", 15));
            }
            else
            {
                if (_Speed>0)
                {
                    RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_VAR, string.Format("{0}\n{1}={2}\n", UnitID, "PAR_PRINTING_SPEED", _Speed.ToString()));
                }
            }
        }

    }
}
