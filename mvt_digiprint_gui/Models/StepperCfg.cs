using RX_Common;
using RX_DigiPrint.Services;
using System;

namespace RX_DigiPrint.Models
{
    public class StepperCfg : RxBindable
    {       
        //--- Constructor ------------------------------------------
        public StepperCfg()
        {
        }

        //--- Property Changed ---------------------------------------
        private bool _Init    = false;
        private bool _Changed = false;
        public bool Changed
        {
            get { return _Changed && _Init; }
            set { SetProperty(ref _Changed, value); }
        }

        //--- Property DevelopmentMachine ---------------------------------------
        private bool _DevelopmentMachine;
        public bool DevelopmentMachine
        {
            get { return _DevelopmentMachine; }
            set { Changed |= SetProperty(ref _DevelopmentMachine, value); }
        }

        //--- Property RefHeight ---------------------------------------
        private double _RefHeight;
        public double RefHeight
        {
            get { return _RefHeight; }
            set { Changed|=SetProperty(ref _RefHeight, value); }
        }      
        
        //--- Property PrintHeight ---------------------------------------
        private double _PrintHeight;
        public double PrintHeight
        {
            get { return _PrintHeight; }
//          set { Changed|=SetProperty(ref _PrintHeight, value); }
            set { SetProperty(ref _PrintHeight, value); }
        }      

        //--- Property WipeHeight ---------------------------------------
        private double _WipeHeight;
        public double WipeHeight
        {
            get { return _WipeHeight; }
            set { Changed|=SetProperty(ref _WipeHeight, value); }
        }      

        //--- Property CapHeight ---------------------------------------
        private double _CapHeight;
        public double CapHeight
        {
            get { return _CapHeight; }
            set { Changed|=SetProperty(ref _CapHeight, value); }
        }      

        //--- Property AdjustPos ---------------------------------------
        private double _AdjustPos;
        public double AdjustPos
        {
            get { return _AdjustPos; }
            set { Changed|=SetProperty(ref _AdjustPos, value); }
        }

        //--- Property WipeDelay ---------------------------------------
        private int _WipeDelay;
        public int WipeDelay
        {
            get { return _WipeDelay; }
            set { Changed|=SetProperty(ref _WipeDelay, value); }
        }

        //--- Property WipeSpeed ---------------------------------------
        private int _WipeSpeed;
        public int WipeSpeed
        {
            get { return _WipeSpeed; }
            set { Changed|=SetProperty(ref _WipeSpeed, value); }
        }

        
        public TcpIp.SRobotOffsets[] Robot = new TcpIp.SRobotOffsets[4];

        //--- SetStepperCfg ----------------------------------------
        public void SetStepperCfg(TcpIp.SStepperCfgMsg msg)
        {
            DevelopmentMachine = msg.development_machine == 1;
            RefHeight   = (msg.ref_height   /1000.0);
            PrintHeight = (msg.print_height /1000.0);
            WipeHeight  = (msg.wipe_height  /1000.0);
            CapHeight   = (msg.cap_height   /1000.0);
            AdjustPos   = (msg.adjust_pos   /1000.0);
            WipeDelay   = msg.wipe_delay;
            WipeSpeed   = msg.wipe_speed;
            Robot       = msg.robot;
            OnPropertyChanged("Robot");
            Changed=false;
            _Init = true;
        }

        //--- SendStepperCfg ------------------------------------------------------
        public void SendStepperCfg()
        {
            TcpIp.SStepperCfgMsg msg = new TcpIp.SStepperCfgMsg();

            msg.ref_height   = (Int32)(RefHeight   * 1000);
            msg.print_height = (Int32)(PrintHeight * 1000);
            msg.wipe_height  = (Int32)(WipeHeight  * 1000);
            msg.cap_height   = (Int32)(CapHeight   * 1000);
            msg.adjust_pos   = (Int32)(AdjustPos   * 1000);
            msg.wipe_delay   = WipeDelay;
            msg.wipe_speed   = WipeSpeed;
            
            msg.robot        = Robot;

            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SET_STEPPER_CFG, ref msg);
        }        
    }
}
