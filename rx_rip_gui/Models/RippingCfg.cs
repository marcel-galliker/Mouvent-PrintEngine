using RX_Common;
using rx_rip_gui.External;
using rx_rip_gui.Services;
using System;
using System.Xml;

namespace rx_rip_gui.Models
{
    public class RippingCfg : RxBindable, ICloneable
    {
        //--- Constructor ----------------------------
        public RippingCfg()
        {
        }

        //--- Clone -------------------------------------------
        public object Clone()
        {
            return this.MemberwiseClone();
        }

        //--- Cfg
        public SRippingCfg Cfg
        {
            get 
            { 
                SRippingCfg cfg;

                cfg.outputProfileName   = OutputProfileName;
                cfg.renderIntent        = RenderIntent;
                cfg.blackPreserve       = BlackPreserve;   
                cfg.blackPtComp         = Convert.ToByte(BlackPtComp);     
                cfg.simuOverprint       = Convert.ToByte(SimuOverprint);      
                return cfg;
            }

            set 
            { 
                OutputProfileName   = value.outputProfileName;
                RenderIntent        = value.renderIntent;
                BlackPreserve       = value.blackPreserve;
                BlackPtComp         = value.blackPtComp!=0;
                SimuOverprint       = value.simuOverprint!=0;
            }
        }

        //--- Property ProfileName ---------------------------------------
        private string _ProfileName="profile";
        public string OutputProfileName
        {
            get { return _ProfileName; }
            set { SetProperty(ref _ProfileName, value); }
        }

        //--- Property RenderIntent ---------------------------------------
        private ERenderIntent _RenderIntent;
        public ERenderIntent RenderIntent
        {
            get { return _RenderIntent; }
            set { SetProperty(ref _RenderIntent, value); }
        }
        
        //--- Property BlackPreserve ---------------------------------------
        private EBlackPreserve _BlackPreserve = EBlackPreserve.bp_undef;
        public EBlackPreserve BlackPreserve
        {
            get { return _BlackPreserve; }
            set { SetProperty(ref _BlackPreserve, value); }
        }

        //--- Property BlackPtComp ---------------------------------------
        private bool _BlackPtComp = false;
        public bool BlackPtComp
        {
            get { return _BlackPtComp; }
            set { SetProperty(ref _BlackPtComp, value); }
        }

        //--- Property SimuOverprint ---------------------------------------
        private bool _SimuOverprint = false;
        public bool SimuOverprint
        {
            get { return _SimuOverprint; }
            set { SetProperty(ref _SimuOverprint, value); }
        }
        
        //--- Save ------------------------------
        public int Save(int xml)
        {
            SRippingCfg cfg = this.Cfg;
            RX_EmbRip.embrip_ripping_cfg(xml, true, ref cfg);
            return 0;
        }

        //--- Load --------------------------------
        public int Load(int xml)
        {
            SRippingCfg cfg = new SRippingCfg();
            RX_EmbRip.embrip_ripping_cfg(xml, false, ref cfg);
            this.Cfg = cfg;            
            return 0;
        }

    }
}
