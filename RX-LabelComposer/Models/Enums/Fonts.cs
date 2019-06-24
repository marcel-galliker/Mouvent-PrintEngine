using RX_Common;
using RX_LabelComposer.External;
using System.Collections;
using System.Collections.Generic;
using System.Text;

namespace RX_LabelComposer.Models.Enums
{

    
    public class FontList : IEnumerable
    {
        private static List<string> _List;

        //--- Constructor ----------------------------------------------------
        public FontList(bool reset)
        {
            if (reset) _List=null;
            if (_List == null)
            {
                _List = new List<string>();
                StringBuilder fontname = new StringBuilder(64);
                int n=0;
                while (RX_Rip.rip_get_font_name(n++, fontname)!=0)
                {
                    _List.Add(fontname.ToString());                
                }
            }
        }

        //--- enumerator --------------------------------------------------------
        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<string>(_List);
        }
    }

    //--- BcFontList -------------------------------------------------------------

    public class BcFontList : IEnumerable
    {
        private static List<string> _List;

        public BcFontList()
        {
            if (_List == null)
            {
                _List = new List<string>();
                _List.Add("OCRB");
                _List.Add("");
            }
        }

        //--- enumerator ------------------------------------------------------
        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<string>(_List);
        }
    }
}