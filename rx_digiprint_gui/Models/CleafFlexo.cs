using RX_Common;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace RX_DigiPrint.Models
{
    public class CleafFlexo : RxBindable
    {
        //--- Property LacCode ---------------------------------------
        private string _LacCode="678";
        public string LacCode
        {
            get { return _LacCode; }
            set { SetProperty(ref _LacCode, value); }
        }

        //--- Property Anilox ---------------------------------------
        private string _Anilox="345";
        public string Anilox
        {
            get { return _Anilox; }
            set { SetProperty(ref _Anilox, value); }
        }

        //--- Property RubberRoll ---------------------------------------
        private string _RubberRoll="123";
        public string RubberRoll
        {
            get { return _RubberRoll; }
            set { SetProperty(ref _RubberRoll, value); }
        }

        //--- LoadFromXml -----------------------------------------
        public void LoadFromXml(XmlTextReader xml)
        {
            foreach(var prop in GetType().GetProperties())
            {
                var attr=xml.GetAttribute(prop.Name);
                        
                if (attr!=null)
                { 
                    try
                    {
                        prop.SetValue(this, TypeDescriptor.GetConverter(prop.PropertyType).ConvertFromString(attr));
                    }
                    catch(Exception)
                    { 
                    }
                }
            }
        }
    }
}
