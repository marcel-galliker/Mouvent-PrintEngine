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
        public virtual void Save(string path)
        {
          //  string path =  Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData)+"\\"+_AppName;
            if (path==null) path = _MyPath;
            if (path==null) path =  Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData)+"\\"+System.Windows.Forms.Application.ProductName;

            if (!Directory.Exists(path))
                Directory.CreateDirectory(path);
            XmlTextWriter xml = new XmlTextWriter(path+"\\user.config", null);
            
            xml.WriteStartDocument();
            xml.WriteStartElement(this.GetType().Name);

            _SaveObject(ref xml, this);

            xml.WriteEndElement();
            xml.WriteEndDocument(); 
            xml.Close();
        }

        private void _SaveObject(ref XmlTextWriter xml, object obj)
		{
            foreach(var prop in obj.GetType().GetProperties())
            {
                var val = prop.GetValue(obj, null);
                if (val!=null)
                {
                    var type = val.GetType();
                    if (type.Namespace.EndsWith("Models"))
					{
                        Console.WriteLine("<{0}", prop.Name);
                        xml.WriteStartElement(prop.Name);
                        _SaveObject(ref xml, val);
                        xml.WriteEndElement();
                        Console.WriteLine("/{0}>", prop.Name);
					}
                    else if (val.GetType().Name.Equals("Boolean[]"))
					{
                        Boolean[] list=val as Boolean[];
                        xml.WriteStartAttribute(prop.Name);
                        for (int n=0; n<list.Length; n++)
                            xml.WriteValue(list[n].ToString()+';');
                        xml.WriteEndAttribute();
                    }
                    else
					{
                        xml.WriteStartAttribute(prop.Name);    
                        xml.WriteValue(val.ToString());
                        xml.WriteEndAttribute();
                        Console.WriteLine("{0}=\"{1}\"", prop.Name, val.ToString());
					}
                }
            }
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
                    Console.WriteLine("{0}: {1}=>>{2}<<", xml.NodeType.ToString(), xml.Name, xml.Value);
                    if (xml.NodeType==XmlNodeType.Element && xml.Name.Equals(this.GetType().Name))
                    { 
                        _LoadObject(ref xml, this);
					}
                }

                xml.Close();
             }
             catch(Exception e)
             {
                 Console.WriteLine(e.Message);
             }
        }

        //--- _LoadObject ------------------------------------------------
        private void _LoadObject(ref XmlTextReader xml, Object obj)
		{
            var objType = obj.GetType();
            foreach(var prop in objType.GetProperties())
            {
                var attr=xml.GetAttribute(prop.Name);
                if (attr!=null)
                { 
                    try
                    {
                        if (prop.PropertyType.Equals(typeof(System.Drawing.Point)))
						{
                            prop.SetValue(obj, _LoadPoint(attr));
						}
                        else if (prop.PropertyType.Name.Equals("Boolean[]"))
                        {
                            string[] list = attr.Split(';');
                            bool[] values = new bool[list.Length-1];
                            for (int i=0; i<list.Length-1; i++)
                                values[i] = Convert.ToBoolean(list[i]);
                            prop.SetValue(obj, values);
                        }
                        else prop.SetValue(obj, TypeDescriptor.GetConverter(prop.PropertyType).ConvertFromString(attr));
                    }
                    catch(Exception e)
                    { 
                        Console.WriteLine("Excapetion {0}", e.Message);
                    }
                }
            }
            while (xml.Read())
			{
                //  Console.WriteLine("{0}: {1}=>>{2}<<", xml.NodeType.ToString(), xml.Name, xml.Value);
                switch(xml.NodeType)
				{
                case XmlNodeType.Element:  
                            var prop = obj.GetType().GetProperty(xml.Name);
                            var element = prop.GetValue(obj);
                            _LoadObject(ref xml, element);
                            break;

                case XmlNodeType.EndElement: return;
                default: 
                        break;
				}
			}
        }

		private System.Drawing.Point _LoadPoint(string attr)
		{
            System.Drawing.Point p = new System.Drawing.Point();
            p.X=Rx.StrToInt32(attr.Substring(attr.IndexOf("X=")));
            p.Y=Rx.StrToInt32(attr.Substring(attr.IndexOf("Y=")));
            return p;
		}
	}
}
