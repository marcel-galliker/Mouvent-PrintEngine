using RX_DigiPrint.Helpers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;
using RX_Common;

namespace RX_DigiPrint.Models
{   
    public class PreviewCash
    {
        private int         _MaxSize = 50;

        private struct _PreviewItem
        {
            public string      path;
            public BitmapImage img;
        };

        private List<_PreviewItem> _List = new List<_PreviewItem>();

        //--- constructor ------------------------------------
        public PreviewCash()
        {
        }

        //--- GetPreview --------------------------------
        public BitmapImage GetPreview(string preview_path)
        {                 
            //---  search in list -------------------
            for (int i=0; i<_List.Count(); i++)
            {
                if (preview_path.Equals(_List[i].path)) 
                {  
                    if  (i>0)
                    { 
                        _List.Insert(0, _List[i]);
                        _List.RemoveAt(i+1);
                    }
                    return _List[0].img;
                }
            }

            //--- add to list ---------------------------------
            _PreviewItem item = new _PreviewItem();
            item.path = preview_path;
            if (RxPing.HostReachable(preview_path))
            {
                try { item.img = new BitmapImage(new Uri(preview_path));}
                catch(Exception) {   item.img = null; }
            }
            
            _List.Insert(0, item);
            if (_List.Count==_MaxSize) _List.RemoveAt(_MaxSize-1);

            return item.img;
        }
    }
}
