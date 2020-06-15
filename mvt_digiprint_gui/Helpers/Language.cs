using System;
using System.Resources;

namespace RX_DigiPrint.Helpers
{
    public class RxLanguage
    {
        private static ResourceManager _rm;
        private static string DefaultLang="english";

        //--- constructor ---------------------------------
        public RxLanguage()
        {
            Language = DefaultLang;
        }
        
        //--- Language --------------------------------------
        private string _Language;
        public string Language
        {
            get { return _Language; }
            set 
            { 
                if (!value.Equals(_Language))
                {
                    _Language = value; 
                    string filename = string.Format("RX_DigiPrint.Resources.Language.{0}", _Language);
                    _rm = new ResourceManager(filename, typeof(App).Assembly);
                }
            }
        }
        
        //--- GetString ----------------------------------
        public string GetString(string name)
        {
            try
            {
                return _rm.GetString(name);
            }
            catch (Exception)
            {
                return name;
            }
        }

        //--- GetString -----------------------------------
        public string GetString(string chapter, string name)
        {
            return GetString(chapter+"."+name);
        }
    }
}
