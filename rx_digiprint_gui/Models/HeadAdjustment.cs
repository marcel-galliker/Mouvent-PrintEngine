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

        //--- Property PrintbarNo ---------------------------------------
        private double _PrintbarNo = 0;
        public double PrintbarNo
        {
            get { return _PrintbarNo; }
            set { SetProperty(ref _PrintbarNo, value); }
        }

        //--- Property PrintbarNo ---------------------------------------
        private double _HeadNo = 0;
        public double HeadNo
        {
            get { return _HeadNo; }
            set { SetProperty(ref _HeadNo, value); }
        }

        //--- Property Stitch ---------------------------------------
        private double _Steps=0;
        public double Steps    
        {
            get { return _Steps; }
            set { SetProperty(ref _Steps, value); }
        }

        //--- Property Axis ---------------------------------------
        private double _Axis=0;
        public double Axis
        {
            get { return _Axis; }
            set { SetProperty(ref _Axis, value); }
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

        //--- Property Running ---------------------------------------
        private bool _Running;
        public bool Running
        {
            get { return _Running; }
            set { SetProperty(ref _Running, value); }
        }

        //--- Property Done ---------------------------------------
        private bool _Done;
        public bool Done
        {
            get { return _Done; }
            set { SetProperty(ref _Done, value); }
        }
        
        //--- Adjust -------------------------------------
        public void Adjust()
        {
            Progress= 0;
            Done    = false;
            Running = true;

            TcpIp.SHeadAdjustmentMsg msg = new  TcpIp.SHeadAdjustmentMsg();

            msg.printbarNo = (Int32)(PrintbarNo);
            msg.headNo     = (Int32)(HeadNo);
            msg.axis       = (Int32)(Axis);
            msg.steps      = (Int32)(Steps);
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_ROB_ADJUST, ref msg);
        }        

    }
}
