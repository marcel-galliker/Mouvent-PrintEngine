using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Media;

namespace RX_DigiPrint.Models
{
    public class Material : RxBindable, ICloneable
    {
        class SParVal
        {
            public string ID;
            public StringBuilder val;
        };

        private string          _UnitID = "Application.GUI_00_001_Main";
        private List<SParVal>    _Par;

        //--- constructor -------------------------------
        public Material()
        {
        }
        
        //--- constructor -------------------------------
        public Material(string msg)
        {
            int i;
            string[] lines=msg.Split('\n');
            Name = lines[0];
            _Par = new List<SParVal>();
            for (i=1; i<lines.Length; i++)
            {
                string[] par=lines[i].Split('=');
                if (par.Length<2) break;
                _Par.Add(new SParVal(){ID=par[0],   val=new StringBuilder(32).Append(par[1])});
                
                if (par[0].Equals("PAR_HEAD_HEIGHT"))  
                    RxGlobals.MaterialXML.PrintHeight = Rx.StrToDouble(par[1]);
            }
        }

        //--- Clone -------------------------------------------
        public object Clone()
        {
            return this.MemberwiseClone();
        }

        //--- RxClone ------------------------------------
        public Material RxClone()
        {
            return (Material)this.MemberwiseClone();
        }

        //--- Property Name ---------------------------------------
        private string _Name;
        public string Name
        {
            get { return _Name; }
            set { SetProperty(ref _Name, value); }
        }
        
        //--- Property Value ---------------------------------------
        public string Value(string id)
        {
            int i;
            if (id.StartsWith("XML_"))
                i=0;
            if (_Par==null) return null;
            for (i=0; i<_Par.Count; i++)
            {
                if (_Par[i].ID.Equals(id))
                {
                    try { return _Par[i].val.ToString(); }
                    catch(Exception) 
                    {
                        return null; 
                    }
                }
            }
            return null;
        }
        
        //---- SaveValue ---------------------------------
        public void SaveValue(string id, string val)
        {
            if (val==null) return;
            int i;
            if (_Par==null) _Par=new List<SParVal>();
            for (i=0; i<_Par.Count; i++)
            {
                if (_Par[i].ID.Equals(id)) 
                {
                    _Par[i].val.Clear();
                    _Par[i].val.Append(val);
                    return;
                }
            }
            _Par.Add(new SParVal(){ID=id,   val=new StringBuilder(32).Append(val)});
        }

        //--- _parstr ------------------------------------------------------------
        private string _parstr(string id, object val)
        {
            return string.Format("{0}={1}\n", id, val.ToString());
        }

        //--- Delete ---------------------------------------------------------
        public void Delete()
        {
            RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_DEL_MATERIAL, Name);    
        }

        //--- Send ----------------------------------------------
        public void Send(UInt32 cmd)
        {
            int i;
            StringBuilder str = new StringBuilder(2048);
            if (cmd==TcpIp.CMD_PLC_SET_VAR)     str.Append(string.Format("{0}\n", _UnitID));
            else                                str.Append(string.Format("{0}\n", Name));

            for(i=0; i<_Par.Count; i++)
            {
                str.Append(string.Format("{0}={1}\n", _Par[i].ID, _Par[i].val));
            }
            
            RxGlobals.RxInterface.SendMsgBuf(cmd, str.ToString());

            if (cmd==TcpIp.CMD_PLC_SET_VAR) RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_VAR, _parstr("CMD_SET_PARAMETER", 1));
        }

    }
}
