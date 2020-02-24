using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Services;
using System;
using System.IO;

namespace RX_DigiPrint.Models
{
    public class License : RxBindable
    {
        public static Action OnLicenseChanged;

        public License()
        {
        }

        //--- Property Manufacturer ---------------------------------------
        private string _Manufacturer;
        public string Manufacturer
        {
            get { return _Manufacturer; }
            set { SetProperty(ref _Manufacturer, value); }
        }

        //--- Property Model ---------------------------------------
        private string _Model;
        public string Model
        {
            get { return _Model; }
            set { SetProperty(ref _Model, value); }
        }

        //--- Property Serial ---------------------------------------
        private string _Serial;
        public string Serial
        {
            get { return _Serial; }
            set { SetProperty(ref _Serial, value); }
        }        

        //--- Property User ---------------------------------------
        private string _User;
        public string User
        {
            get { return _User; }
            set { SetProperty(ref _User, value); }
        }

        //--- Property ServiceLevel ---------------------------------------
        private EUserType _ServiceLevel;
        public EUserType ServiceLevel
        {
            get 
            { 
                return EUserType.usr_mouvent;
                if (_ServiceLevel<EUserType.usr_supervisor) return EUserType.usr_supervisor;
                else                                        return _ServiceLevel; 
            }

            set { SetProperty(ref _ServiceLevel, value); }
        }        
        
        //--- Property Expiring ---------------------------------------
        private DateTime _Expiring;
        public DateTime Expiring
        {
            get { return _Expiring; }
            set { SetProperty(ref _Expiring, value); }
        }
                
        //--- Valid --------------------------------
        private bool _Valid;
        public bool Valid
        {
            get { return _Valid; }
            set { _Valid = value; }
        }

        //--- Property Code -------------------------------    --------
        public string Code
        {
            set 
            {
                try
                {                
                    string license = RxEncypt.Decrypt(value, RxBtDef.LicPwd);
                    string licens1 = license.Remove(license.Length-1);
                    //  license content
                    //  [0]    AppGlobals.DeviceId.Manufacturer   +"\n"+
                    //  [1]    AppGlobals.DeviceId.Model          +"\n"+
                    //  [2]    AppGlobals.DeviceId.Serial         +"\n"+
                    //  [3]    AppGlobals.DeviceId.UserName       +"\n"+
                    //------------------------------------------------
                    //  [4]    Service_Level    
                    //  [5]    Expiring Data

                    licens1 += RxBtDef.StrCrc(licens1);
                    if (license.Equals(licens1))
                    {
                        string[] lic=license.Split('\n');
                        Manufacturer = lic[0];
                        Model        = lic[1];
                        Serial       = lic[2];
                        User         = lic[3];

                        ServiceLevel = (EUserType) Rx.StrToInt32(lic[4]);

                        string[] d   = lic[5].Split('.');
                        int day=Rx.StrToInt32(d[0]);
                        int month=Rx.StrToInt32(d[1]);
                        int year = Rx.StrToInt32(d[2])/10;
                        _Expiring = new DateTime(year, month, day);
                        if (_Expiring>=DateTime.Today)
                        {
                            Valid = true;
                        }
                        else
                        {
                            Valid=false;
                            RxGlobals.Events.AddItem(new LogItem("License expired"){LogType=ELogType.eErrWarn});                
                        }
                    }
                    else
                    {
                        Valid=false;
                        RxGlobals.Events.AddItem(new LogItem("License invalid"){LogType=ELogType.eErrWarn});                
                        return;
                    }
                }
                catch(Exception)
                {
                    Valid=false;
                    RxGlobals.Events.AddItem(new LogItem("License invalid"){LogType=ELogType.eErrWarn});                
                    return;
                }
            }

            get
            {
                string str = Manufacturer + "\n"
                           + Model + "\n"
                           + Serial + "\n"
                           + User + "\n"
                           + Convert.ToInt32(ServiceLevel).ToString()+"\n"
                           + Expiring.Day.ToString()+"."+Expiring.Month.ToString()+"."+Expiring.Year.ToString();
                str += RxBtDef.StrCrc(str);
                return RxEncypt.Encrypt(str, RxBtDef.LicPwd);
            }
        }
        
        //--- Update ---------------------------------------
        public void Update()
        {
            License act=RxGlobals.License;
            try
            {
                License lic = new License();
//              string filepath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments), RxBtDef.LicFileName);
                string filepath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.UserProfile)+"\\Downloads", RxBtDef.LicFileName);
                lic.Code = System.IO.File.ReadAllText(filepath);
                if (!lic.Valid) 
                {
                    RxGlobals.License = new License(); 
                    return;
                }
                int diff = DateTime.Compare(lic.Expiring, DateTime.Today);
                if      (diff==0) RxGlobals.License = lic;
                else if (diff<0)  RxGlobals.License = new License(); 
                else
                {
                    lic.Expiring = DateTime.Today;
                    string code = lic.Code;
                    System.IO.File.WriteAllText(filepath, code);
                    string code1 = System.IO.File.ReadAllText(filepath);
                    if (code.Equals(code1)) RxGlobals.License = lic;
                    else                    RxGlobals.License = new License(); 
                }
            }
            catch(Exception)
            {
                RxGlobals.License=new License();
            };
            if (OnLicenseChanged!=null && act.ServiceLevel != RxGlobals.License.ServiceLevel) 
                OnLicenseChanged();
        }
    }
}
