using RX_Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
    public class WlanNetwork : RxBindable
    {
        public WlanNetwork()
        {
        }

        //--- Property Name ---------------------------------------
        private string _Name;
        public string Name
        {
            get { return _Name; }
            set { SetProperty(ref _Name, value); }
        }

        //--- Property HasProfile ---------------------------------------
        private bool _HasProfile;
        public bool HasProfile
        {
            get { return _HasProfile; }
            set { SetProperty(ref _HasProfile, value); }
        }
        
        //--- Property SignalQuality ---------------------------------------
        private UInt32 _SignalQuality;
        public UInt32 SignalQuality
        {
            get { return _SignalQuality; }
            set { SetProperty(ref _SignalQuality, value); }
        }
        
        //--- Property Connected ---------------------------------------
        private bool _Connected;
        public bool Connected
        {
            get { return _Connected; }
            set { SetProperty(ref _Connected, value); }
        }
        
    }
}
