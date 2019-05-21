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

        //--- Property Stitch ---------------------------------------
        private double _Stitch=0;
        public double Stitch    
        {
            get { return _Stitch; }
            set { SetProperty(ref _Stitch, value); }
        }

        //--- Property Angle ---------------------------------------
        private double _Angle=0;
        public double Angle
        {
            get { return _Angle; }
            set { SetProperty(ref _Angle, value); }
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
        public void Adjust(int inkSupplyNo, int headNo)
        {
            Progress= 0;
            Done    = false;
            Running = true;

            TcpIp.SHeadAdjustmentMsg msg = new  TcpIp.SHeadAdjustmentMsg();

            msg.inkSupplyNo = inkSupplyNo;
            msg.headNo      = headNo;
            msg.angle       = (Int32)(Angle*1000);
            msg.stitch      = (Int32)(Stitch*1000);
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_CLN_ADJUST, ref msg);
        }        

    }
}
