using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Xml;

namespace RxWfTool.Helpers
{
    class RxXml
    {

            //--- SaveProperty --------------------------------------------------------
//        static public void SaveProperty(XmlTextWriter xml, object obj, String propertyName)
        static public void SaveProperty(XmlWriter xml, object obj, String propertyName)
        {
            PropertyInfo property   = obj.GetType().GetProperty(propertyName);
            object       value      = property.GetValue(obj, null);

            if (value!=null)
            {
                xml.WriteStartAttribute(propertyName);
                if (property.PropertyType.ToString().Equals("System.Int32[]"))
                {
                    int[] val=value as int[];
                    StringBuilder str = new StringBuilder(256);
                    for (int i=0; i<val.Count(); i++)
                    {
                        str.Append(val[i].ToString()+" ");
                    }
                    xml.WriteValue(str.ToString());
                }
                else
                {
                    StringBuilder str= new StringBuilder(128);
                    foreach(char ch in value.ToString())
                    {
                        if (ch<0x80) str.Append(ch);
                        else 
                        {
                            int i = ch;
                            str.Append(string.Format("&#{0};", i));
                        }
                    }
                    xml.WriteRaw(str.ToString());
                }
                xml.WriteEndAttribute();
            }
        }

        //--- LoadProperty --------------------------------------------------------
        static public void LoadProperty(XmlTextReader xml, object obj, String propertyName)
        {
            PropertyInfo property   = obj.GetType().GetProperty(propertyName);
            string attribute        = xml.GetAttribute(propertyName);

            if (attribute==null) 
                return;

            if (property.PropertyType.ToString().Equals("System.String")) 
                property.SetValue(obj, attribute);

            else if (property.PropertyType.ToString().Equals("System.Windows.Media.Color"))
                property.SetValue(obj, (Color)ColorConverter.ConvertFromString(attribute));

            else if (property.PropertyType.ToString().Equals("System.Int32"))       
                property.SetValue(obj, System.Convert.ToInt32(attribute));

            else if (property.PropertyType.ToString().Equals("System.Int32[]"))
            {
                string[] str = attribute.Split(' ');
                int[] val  = new int[str.Count()-1];
                for (int i=0; i<str.Count()-1; i++)
                    val[i]=System.Convert.ToInt32(str[i]);
                property.SetValue(obj, val);
            }
            else throw new System.Exception("Type not implemented");
        }
    }
}
