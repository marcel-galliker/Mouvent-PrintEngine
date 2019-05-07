using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace RX_DigiPrint.Models
{
    public class PageNumber  : RxBindable
    {
        //--- constructor ------------------------------
        public PageNumber(XmlTextReader xml)
        {
            for  (int i=0; i<xml.AttributeCount; i++)
            {
                xml.MoveToAttribute(i);
                var prop = GetType().GetProperty(xml.Name);
                prop.SetValue(this, TypeDescriptor.GetConverter(prop.PropertyType).ConvertFromString(xml.Value));
            }
            xml.MoveToElement();
        }

        //--- constructor ------------------------------
        public PageNumber(TcpIp.sPageNumber pn)
        {
            Enabled  = (pn.enabled!=0);
            Number   = pn.number;
            FontSize = pn.fontSize;
            ImgDist  = pn.imgDist/1000.0;
        }

        //--- Property Changed ---------------------------------------
        private bool _Changed = false;
        public bool Changed
        {
            get { return _Changed; }
            set { SetProperty(ref _Changed, value); }
        }
        
        //--- ToMsg -------------------------------------
        public void ToMsg(ref TcpIp.sPageNumber pn)
        {
            pn.enabled  = Convert.ToUInt32(Enabled);
            pn.number   = Number;
            pn.fontSize = FontSize;
            pn.imgDist  = Convert.ToUInt32(ImgDist*1000.0+0.5);
        }

        //--- Property Enabled ---------------------------------------
        private bool _Enabled;
        public bool Enabled
        {
            get { return _Enabled; }
            set { Changed |= SetProperty(ref _Enabled, value); }
        }

        //--- Property Number ---------------------------------------
        private UInt32 _Number;
        public UInt32 Number
        {
            get { return _Number; }
            set { Changed |= SetProperty(ref _Number, value); }
        }

        //--- Property FontSize ---------------------------------------
        private UInt32 _FontSize;
        public UInt32 FontSize
        {
            get { return _FontSize; }
            set { Changed |= SetProperty(ref _FontSize, value); }
        }

        //--- Property ImgDist ---------------------------------------
        private Double _ImgDist;
        public Double ImgDist
        {
            get { return _ImgDist; }
            set { Changed |= SetProperty(ref _ImgDist, value); }
        }
     
        //--- save_property ------------------------
        private void _save_property(XmlTextWriter xml, string propname)
        {
            var prop = GetType().GetProperty(propname);
            xml.WriteStartAttribute(prop.Name);
            xml.WriteValue(prop.GetValue(this).ToString());
            xml.WriteEndAttribute();
        }

        //--- SaveDefaults -----------------------
        public void SaveDefaults(XmlTextWriter xml)
        {
            xml.WriteStartElement("PageNumber");
            
            _save_property(xml, "Enabled");
            _save_property(xml, "Number");
            _save_property(xml, "FontSize");
            _save_property(xml, "ImgDist");

            xml.WriteEndElement();
        }
    }
}
