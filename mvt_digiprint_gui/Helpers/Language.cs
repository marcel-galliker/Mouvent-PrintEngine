using System;
using System.Resources;

namespace RX_DigiPrint.Helpers
{
    public class RxLanguage
    {
        //--- constructor ---------------------------------
        public RxLanguage()
        {
        }

        //--- GetString ----------------------------------
        public string GetString(string name)
        {
            if (TranslationSource.has(name))
            {
                return TranslationSource.Instance[name];
            }
            return null;
        }
        
        //--- GetString -----------------------------------
        public string GetString(string chapter, string name)
        {
            return GetString(chapter + "." + name);
        }
    }
}
