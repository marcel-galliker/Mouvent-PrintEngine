using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
    public class Chiller : RxBindable
    {
        //--- constructor ---
        public Chiller()
        {
        }

        //--- Property Enabled ---------------------------------------
        private bool _Enabled=false;
        public bool Enabled
        {
            get { return _Enabled; }
            set { SetProperty(ref _Enabled, value); }
        }

        //--- Property Running ---------------------------------------
        private bool _Running=true;
        public bool Running
        {
            get { return _Running; }
            set { SetProperty(ref _Running, value); }
        }

        //--- Property Temp ---------------------------------------
        private Int32 _Temp;
        public Int32 Temp
        {
            get { return _Temp; }
            set { SetProperty(ref _Temp, value); }
        }

        //--- Property TempSet ---------------------------------------
        private Int32 _TempSet;
        public Int32 TempSet
        {
            get { return _TempSet; }
            set { SetProperty(ref _TempSet, value); }
        }        

        //--- Property Pressure ---------------------------------------
        private Int32 _Pressure;
        public Int32 Pressure
        {
            get { return _Pressure; }
            set { SetProperty(ref _Pressure, value); }
        }

        //--- Property Resistivity ---------------------------------------
        private Int32 _Resistivity;
        public Int32 Resistivity
        {
            get { return _Resistivity; }
            set { SetProperty(ref _Resistivity, value); }
        }
        
        //--- Update -----------------------------------
        public void Update(TcpIp.SChillerStat msg)
        {
            Enabled     = msg.enabled!=0;
            Running     = (msg.running!=0) || (msg.enabled==0);
            TempSet     = msg.tempSet;
            Temp        = msg.temp;
            Pressure    = msg.pressure;
            Resistivity = msg.resistivity;
        }

    }
}
