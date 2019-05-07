using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;

namespace RX_DigiPrint.Models
{
    public class StepperMotor : RxBindable
    {
        public StepperMotor()
        {
        }

        //--- Property No ---------------------------------------
        private int _No;
        public int No
        {
            get { return _No; }
            set { SetProperty(ref _No, value); }
        }
        
        //--- Property MotorPos ---------------------------------------
        private double _MotorPos;
        public double MotorPos
        {
            get { return _MotorPos; }
            set { SetProperty(ref _MotorPos, value); }
        }

        //--- Property EncoderPos ---------------------------------------
        private double _EncoderPos;
        public double EncoderPos
        {
            get { return _EncoderPos; }
            set { SetProperty(ref _EncoderPos, value); }
        }

        //--- Property FirstPageOk ---------------------------------------
        private Color _EncoderColor = Colors.Transparent;
        public Color EncoderColor
        {
            get { return _EncoderColor; }
            set { SetProperty(ref _EncoderColor, value); }
        }

        //--- Property EndSwitch ---------------------------------------
        private bool _EndSwitch;
        public bool EndSwitch
        {
            get { return _EndSwitch; }
            set { SetProperty(ref _EndSwitch, value); }
        }

        //--- Property State ---------------------------------------
        private UInt32 _State;
        public UInt32 State
        {
            get { return _State; }
            set { SetProperty(ref _State, value); }
        }

        //--- DoTest -------------------------------------
        public void DoTest(bool up)
        {
            TcpIp.SStepperMotorTestMsg msg = new TcpIp.SStepperMotorTestMsg();
            msg.boardNo = 0;
            msg.motorNo = No;
            if (up) msg.microns = 500;
            else    msg.microns = -500;
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_STEPPER_TEST, ref msg);
        }
        
        //--- SendBt ----------------------------------------------------
        public void SendBt(RxBtClient client)
        {
            RxBtDef.SBtStepperStatMsg msg = new RxBtDef.SBtStepperStatMsg();
            msg.no                  = No;
            msg.status.motorPos     = MotorPos;
            msg.status.encoderPos   = EncoderPos;
            msg.status.encoderColor = Rx.ToArgb(EncoderColor);
            msg.status.endSwitch    = Convert.ToUInt32(EndSwitch);
            msg.status.state        = State;
            if (client==null) RxGlobals.Bluetooth.SendMsg(RxBtDef.BT_REP_STEPPER_STATE, ref msg);
            else              client.SendMsg(RxBtDef.BT_REP_STEPPER_STATE, ref msg);
        }

    }
}
