using RX_Common;
using rx_rip_gui.External;
using rx_rip_gui.Services;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace rx_rip_gui.Models
{
    public class PrEnv:RxBindable, ICloneable
    {
        //---  Constructor -------------------------
        public PrEnv()
        {
        }

        //--- _dir ---------------------
        public string Dir(string name)
        {
            if (name==null || name.Equals("")) return rip_def.RX_PRENV_ROOT + Name;
            return rip_def.RX_PRENV_ROOT + name;
        }

        //--- _path ---------------------
        private string _path(string name)
        {
            string path = rip_def.RX_PRENV_ROOT + Name;
            return Dir(name) + "/" + name + rip_def.PRENV_EXT;
        }

        //--- Cfg -------------------------------------
        public SPrintEnv Cfg
        {
            get 
            { 
                SPrintEnv cfg = new SPrintEnv();

                cfg.name        = Name;
                if (Screening!=null)    cfg.screening   = Screening.Cfg;
                if (Ripping!=null)      cfg.ripping     = Ripping.Cfg;
                return cfg;
            }

            set 
            { 
                Name        = value.name;
                Load(Name);
           //     Screening   = new ScreeningCfg(){Cfg=value.screening};
           //     Ripping     = new RippingCfg(){Cfg=value.ripping};
            }
        }


        //--- Property Name ---------------------------------------
        private string _Name="new print environment";
        public string Name
        {
            get { return _Name; }
            set { SetProperty(ref _Name, value);}
        }

        //--- Property Screening ---------------------------------------
        private ScreeningCfg _Screening =  new ScreeningCfg();
        public ScreeningCfg Screening
        {
            get { return _Screening; }
            set { SetProperty(ref _Screening, value); }
        }

        //--- Property Ripping ---------------------------------------
        private RippingCfg _Ripping = new RippingCfg();
        public RippingCfg Ripping
        {
            get { return _Ripping; }
            set { SetProperty(ref _Ripping, value); }
        }    

        //--- Property Changed ---------------------------------------
        private bool _Changed = false;
        public bool Changed
        {
            get { return _Changed; }
            set { SetProperty(ref _Changed, value); }
        }

        //--- Clone -------------------------------------------
        public object Clone()
        {
            PrEnv clone = (PrEnv)this.MemberwiseClone();
            clone.Screening = (ScreeningCfg)Screening.Clone();
            clone.Ripping   = (RippingCfg)Ripping.Clone();
            return clone;
        }

        //--- Save ---------------------------------------------
        public void Save()
        {   
            Directory.CreateDirectory(Dir(Name));
            
            int doc = RX_EmbRip.embrip_create_xml();
            Screening.Save(doc);
            Ripping.Save(doc);
            
            RX_EmbRip.embrip_save_xml(doc, _path(Name));

            Changed = false;
        }

        //--- Load --------------------------------------------
        public bool Load(string name)
        {
            try
            {
                int doc = RX_EmbRip.embrip_open_xml(_path(name));
                if (doc!=0)
                {
                    Screening.Load(doc);
                    Ripping.Load(doc);
                    RX_EmbRip.embrip_close_xml(doc);
                }

                return true;
            }
            catch(Exception)
            {
                return false;
            }
        }

        //--- Delete --------------------------------------
        public void Delete()
        {
            DirectoryInfo directory = new DirectoryInfo(Dir(Name));
            foreach(System.IO.FileInfo file in directory.GetFiles()) file.Delete();
           // foreach(System.IO.DirectoryInfo subDirectory in directory.GetDirectories()) subDirectory.Delete(true);
           directory.Delete();
        }
        
    }
}
    