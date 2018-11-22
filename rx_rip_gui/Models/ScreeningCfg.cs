using RX_Common;
using rx_rip_gui.Services;
using rx_rip_gui.External;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace rx_rip_gui.Models
{
    public class ScreeningCfg : RxBindable, ICloneable
    {
        //--- Constructor ----------------------
        public ScreeningCfg()
        {
        }

        //--- Clone -------------------------------------------
        public object Clone()
        {
            return this.MemberwiseClone();
        }

        //--- Cfg -------------------------------------
        public SScreeningCfg Cfg
        {
            get 
            { 
                SScreeningCfg cfg;

                cfg.profileName   = ProfileName;
                cfg.mode          = Mode;
                return cfg;
            }

            set 
            { 
                ProfileName   = value.profileName;
                Mode          = value.mode;
            }
        }

        //--- Property Mode ---------------------------------------
        private EScreeningMode _Mode = EScreeningMode.sm_1;
        public EScreeningMode Mode
        {
            get { return _Mode; }
            set { SetProperty(ref _Mode, value); }
        }

        //--- Property ProfileName ---------------------------------------
        private string _ProfileName="profile";
        public string ProfileName
        {
            get { return _ProfileName; }
            set { SetProperty(ref _ProfileName, value); }
        }

        //--- Save ------------------------------
        public int Save(int xml)
        {
            SScreeningCfg cfg = this.Cfg;
            RX_EmbRip.embrip_screening_cfg(xml, true, ref cfg);
            return 0;
        }

        //--- Load --------------------------------
        public int Load(int xml)
        {
            SScreeningCfg cfg = new SScreeningCfg();
            RX_EmbRip.embrip_screening_cfg(xml, false, ref cfg);
            this.Cfg = cfg;            
            return 0;
        }
    }
}
