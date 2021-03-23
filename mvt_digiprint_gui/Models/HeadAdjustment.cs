using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
    public class HeadAdjustment : RxBindable
    {
        public HeadAdjustment()
        {
        }
        
        //--- Reset ----------------------------------------------
        public void Reset()
        {
            Running  = false;
            Done     = false;
            Progress = 0;
        }

		//--- Property StepperNo ---------------------------------------
		private int _StepperNo;
		public int StepperNo
		{
			get { return _StepperNo; }
			set { SetProperty(ref _StepperNo,value); }
		}

		//--- Property PrintbarNo ---------------------------------------
		private double _PrintbarNo = 0;
        public double PrintbarNo
        {
            get { return _PrintbarNo; }
            set
            {
                SetProperty(ref _PrintbarNo, value);
            }
        }

        //--- Property HeadNo ---------------------------------------
        private double _HeadNo = 0;
        public double HeadNo
        {
            get { return _HeadNo; }
            set
            {
                SetProperty(ref _HeadNo, value);
            }
        }

        //--- Property Steps ---------------------------------------
        private double _Steps=0;
        public double Steps    
        {
            get { return _Steps; }
            set 
            {
                SetProperty(ref _Steps, value); 
            }
        }

        //--- Property Axis ---------------------------------------
        private double _Axis=0;
        public double Axis
        {
            get { return _Axis; }
            set 
            { 
                SetProperty(ref _Axis, value); 
            }
        }

        //--- Property Progress ---------------------------------------
        private int _Progress=0;
        public int Progress
        {
            get { return _Progress; }
            set 
            { 
                SetProperty(ref _Progress, value);
                if (value>=100)
                {
                    Running=false;
                    Done=true;
                }
            }
        }

		//--- Property State ---------------------------------------
		private string _State;
		public string State
		{
			get { return _State; }
			set { SetProperty(ref _State,value); }
		}

		//--- Property Running ---------------------------------------
		private bool _Running;
        public bool Running
        {
            get { return _Running; }
            set { 
                    SetProperty(ref _Running, value);
                    if (_Running) State="Running";
                }
        }

        //--- Property Done ---------------------------------------
        private bool _Done;
        public bool Done
        {
            get { return _Done; }
            set { 
                    SetProperty(ref _Done, value); 
                    if (_Done) State="Done";
                }
        }

        private bool _ShowHelp;
        public bool ShowHelp
        {
            get { return _ShowHelp; }
            set { SetProperty(ref _ShowHelp, value); }
        }

        //--- Adjust -------------------------------------
        public void Adjust()
        {
            Progress= 0;
            Done    = false;
            Running = true;

            TcpIp.SHeadAdjustmentMsg msg = new  TcpIp.SHeadAdjustmentMsg();

            msg.printbarNo  = (Int32)(PrintbarNo-1);
            msg.headNo      = (Int32)(HeadNo-1);
            msg.axis        = (Int32)(Axis);
            if (Steps >= 0)
                msg.steps = (Int32)(((Int32)Steps * 6 + (Steps - (Int32)Steps) * 6) + 0.5);
            else
                msg.steps = (Int32)(((Int32)Steps * 6 + (Steps - (Int32)Steps) * 6) - 0.5);
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_HEAD_ADJUST, ref msg);
        }


	}
}
