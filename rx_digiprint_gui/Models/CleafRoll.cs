using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace RX_DigiPrint.Models
{
    public class CleafRoll : RxBindable
    {
        public CleafRoll()
        {
        }

        public CleafRoll(TcpIp.SCleafRoll roll)
        {
            No     = roll.no;
            Length = roll.length;
            user   = roll.user;
            switch(roll.quality)
            {
                case 1:  OK=false; break;
                case 2:  OK=true;  break;
                default: OK=null;  break;
            }
        }

        //--- Property No ---------------------------------------
        private int _No;
        public int No
        {
            get { return _No; }
            set { SetProperty(ref _No, value); }
        }
        
        //--- Property Length ---------------------------------------
        private double _Length;
        public double Length
        {
            get { return _Length; }
            set { SetProperty(ref _Length, value); }
        }

        //--- Property OK ---------------------------------------
        private bool? _OK;
        public bool? OK
        {
            get { return _OK; }
            set { SetProperty(ref _OK, value); }
        }

        //--- Property user ---------------------------------------
        private string _user;
        public string user
        {
            get { return _user; }
            set { SetProperty(ref _user, value); }
        }
        
        //--- SendMsg ---------------------------------------
        public void send_quality(bool? quality)
        {
            TcpIp.SValue value = new TcpIp.SValue();
            value.no = No;
            switch(quality)
            {
                case false: value.value=1; break;
                case true:  value.value=2; break;
                default:    value.value=0; break;
            }

            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_CO_SET_ROLL, ref value);    
        }

    }
}
