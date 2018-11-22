using DigiPrint.Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DigiPrint.Models
{
    public class License : RxBindable
    {
        private DateTime _Expiring;
        public License()
        {
        }

        //--- Property Code ---------------------------------------
        private string _Code;
        public string Code
        {
            get { return _Code; }
            set 
            { 
                SetProperty(ref _Code, value);
                Valid=false;
                Expired=true;
            }
        }

        //--- Property Code ---------------------------------------
        public string PlainCode
        {
            get
            {
                try
                {
                    return  AppGlobals.DeviceId.Manufacturer   +"\n"+
                            AppGlobals.DeviceId.Model          +"\n"+
                            AppGlobals.DeviceId.Serial         +"\n"+
                            AppGlobals.DeviceId.UserName       +"\n"+
                            _Expiring.ToString();                
                }
                catch(Exception){};
                return null;
            }
            
            set 
            {
                try
                {                    
                    string[] data=value.Remove(value.Length-1).Split('\n');
                    Valid = data[0].Equals(AppGlobals.DeviceId.Manufacturer)
                        &&  data[1].Equals(AppGlobals.DeviceId.Model)
                        &&  data[2].Equals(AppGlobals.DeviceId.Serial)
                        &&  data[3].Equals(AppGlobals.DeviceId.UserName);
                    string[] d = data[5].Split('.');
                    int day  = Rx.Str2Int(d[0]);
                    int month= Rx.Str2Int(d[1]);
                    int year = Rx.Str2Int(d[2]);
                    _Expiring = new DateTime(year, month, day);
                    Expired = Valid && (_Expiring<DateTime.Today);
                    if (Valid&!Expired) ServiceLevel=(RxBtDef.EUserType)(Rx.Str2Int(data[4]));
                    else                ServiceLevel=RxBtDef.EUserType.usr_operator;
                }
                catch(Exception)
                {
                    Valid = false;
                }
            }
        }
                
        //--- Property ServiceLevel ---------------------------------------
        private RxBtDef.EUserType _ServiceLevel;
        public RxBtDef.EUserType ServiceLevel
        {
            get { return _ServiceLevel; }
            set { SetProperty(ref _ServiceLevel, value); }
        }

        //--- Property Valid ---------------------------------------
        private bool _Valid;
        public bool Valid
        {
            get { return _Valid; }
            set { SetProperty(ref _Valid, value); }
        }

        //--- Property Expired ---------------------------------------
        private bool _Expired;
        public bool Expired
        {
            get { return _Expired; }
            set { SetProperty(ref _Expired, value); }
        }

    }
}
