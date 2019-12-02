using RX_Common;
using RX_Common.Source;
using RX_DigiPrint.Models.Enums;

namespace RX_DigiPrint.Models
{
    public class Settings : RxSettingsBase
    {
        //--- Constructor --------------------------------------------------
        public Settings()
        {
        }

        //--- Property IP_Addr ---------------------------------------
        private string _IP_Addr = "192.168.200.1";
        public string IP_Addr
        {
            get { return _IP_Addr; }
            set 
            { 
                if (value!=_IP_Addr)
                {
                    _IP_Addr= value;
                    _RippedDataPath = string.Format("\\\\{0}\\ripped-data", value);
                    if (RxGlobals.RxInterface!=null && RxGlobals.RxInterface.Connected) RxNetUse.OpenShare(RippedDataPath, "radex", "radex");
                }
            }
        }

        //--- RippedDataPath -------------------------------------
        private string _RippedDataPath;
        public string RippedDataPath
        {
            get {return _RippedDataPath;}
        }

        //--- Property Name -------------------------------------------
        private string _Name = "Printer";
        public string Name
        {
            get { return _Name; }
            set { _Name = value; }
        }

        //--- Property Units ---------------------------------------
        private EUnits _Units=EUnits.metric;
        public EUnits Units
        {
            get { return _Units; }
            set { SetProperty(ref _Units, value); }
        }
        
    }
}
