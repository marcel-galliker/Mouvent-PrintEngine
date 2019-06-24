using DigiPrint.Common;
using RX_DigiPrint.Services;
using System;
using Xamarin.Forms;

namespace DigiPrint.Models
{
    public class StepperMotor : RxBindable
    {
        private RxBtDef.SBtStepperStat  _act;
        private bool _first=true;
        public StepperMotor(int no)
        {
            _No = no;
        }

        //--- Update --------------------------------------------
        public void Update(RxBtDef.SBtStepperStat msg)
        {
            if (_first||msg.motorPos!=_act.motorPos)           MotorPos    = string.Format("{0:0.000}", msg.motorPos);
            if (_first||msg.encoderPos!=_act.encoderPos)       EncoderPos  = string.Format("{0:0.000}", msg.encoderPos);
            if (_first||msg.encoderColor!=_act.encoderColor)   EncoderColor= Color.FromUint(msg.encoderColor);
            if (_first||msg.endSwitch!=_act.endSwitch)
            {
                if (msg.endSwitch!=0) EndSwitch = ImageSource.FromResource("DigiPrint.Resources.drawable."+"LED_GREEN.ico");
                else                  EndSwitch = ImageSource.FromResource("DigiPrint.Resources.drawable."+"LED_GREY.ico");
            }
            if (_first||msg.state!=_act.state)
            {
                switch(msg.state)
                {
                case 1: State="IDLE";    break;
                case 2: State="MOVE UP"; break;
                case 3: State="MV DOWN"; break;
                case 4: State="BLOCKED"; break;
                default: State="???";     break;
                }
            }
            _act = msg;
            _first=false;
        }

        //--- Property No ---------------------------------------
        private int _No;
        public int No
        {
            get { return _No; }
        }
        
        //--- Property MotorPos ---------------------------------------
        private string _MotorPos;
        public string MotorPos
        {
            get { return _MotorPos; }
            set { SetProperty(ref _MotorPos, value); }
        }

        //--- Property EncoderPos ---------------------------------------
        private string _EncoderPos;
        public string EncoderPos
        {
            get { return _EncoderPos; }
            set { SetProperty(ref _EncoderPos, value); }
        }

        //--- Property FirstPageOk ---------------------------------------
        private Color _EncoderColor = Color.Transparent;
        public Color EncoderColor
        {
            get { return _EncoderColor; }
            set { SetProperty(ref _EncoderColor, value); }
        }

        //--- Property EndSwitch ---------------------------------------
        private ImageSource _EndSwitch;
        public ImageSource EndSwitch
        {
            get { return _EndSwitch; }
            set { SetProperty(ref _EndSwitch, value); }
        }

        //--- Property State ---------------------------------------
        private string _State;
        public string State
        {
            get { return _State; }
            set { SetProperty(ref _State, value);}
        }
    }
}
