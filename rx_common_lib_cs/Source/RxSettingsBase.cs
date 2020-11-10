using System;
using System.IO;
using System.Xml;
using System.ComponentModel;
using System.Diagnostics;


namespace RX_Common
{
    public class RxSettingsBase : RxBindable
    {
      //  private static string _AppName = System.Windows.Forms.Application.ProductName;

        private string _MyPath;

        //--- Constructor ------------------------------
        public RxSettingsBase()
        {
        }

        //--- SaveProperty ------------------------
        static public void SaveProperty(Object obj, XmlTextWriter xml, string propname)
        {
            var prop = obj.GetType().GetProperty(propname);
            var val = prop.GetValue(obj);
            if (val!=null)
            {
                xml.WriteStartAttribute(prop.Name);
                xml.WriteValue(val.ToString());
                xml.WriteEndAttribute();
            }
        }

        //--- LoadProperty --------------------------------
        static public void LoadProperty(Object obj, XmlTextReader xml, string propname)
        {
            var prop = obj.GetType().GetProperty(propname);
            var val=xml.GetAttribute(propname);
            prop.SetValue(obj, TypeDescriptor.GetConverter(prop.PropertyType).ConvertFromString(val));        
        }

        //--- Save -------------------------------
        public void Save(string path)
        {
          //  string path =  Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData)+"\\"+_AppName;
            if (path==null) path = _MyPath;
            if (path==null) path =  Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData)+"\\"+System.Windows.Forms.Application.ProductName;

            if (!Directory.Exists(path))
                Directory.CreateDirectory(path);
            XmlTextWriter xml = new XmlTextWriter(path+"\\user.config", null);
            
            xml.WriteStartDocument();
            xml.WriteStartElement("Settings");

            foreach(var prop in GetType().GetProperties())
            {
                var val = prop.GetValue(this, null);
                if (val!=null)
                {
                    xml.WriteStartAttribute(prop.Name);    
                    xml.WriteValue(val.ToString());
                    xml.WriteEndAttribute();
                }
            }
            xml.WriteEndElement();
            xml.WriteEndDocument(); 
            xml.Close();
        }

        //--- Load -------------------------------
        public void Load(string path)
        {
            if (path==null) path =  Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData)+"\\"+System.Windows.Forms.Application.ProductName;
            if (!Directory.Exists(path)) return;

            _MyPath = path;

            XmlTextReader xml = new XmlTextReader(path+"\\user.config");
            try
            {
                while (xml.Read())
                {
                    if (xml.NodeType==XmlNodeType.Element && xml.Name.Equals("Settings"))
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
                    else
                    {
                    }
                }

                xml.Close();
             }
             catch(Exception e)
             {
                 Console.WriteLine(e.Message);
             }
        }
    }
}
