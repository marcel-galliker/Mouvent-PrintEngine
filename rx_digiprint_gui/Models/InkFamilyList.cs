using RX_Common;
using System;
using System.Collections.ObjectModel;
using System.Linq;

namespace RX_DigiPrint.Models
{
    public class InkFamilyList
    {
        static private ObservableCollection<string> _MyList  = new  ObservableCollection<string>();
                
        public ObservableCollection<string> List
        {
            get { return _MyList; }
        }
        
        //--- AddItem ----------------------------------------
        public void AddItem(string name)
        {
            int idx, cmp;
            for (idx=0; idx<_MyList.Count; idx++)
            {  
                cmp = string.Compare(_MyList[idx], name);
                if (cmp==0) return;
                if (cmp>0) break;
            }
            RxBindable.Invoke(() => _MyList.Insert(idx, name));
        }

        //--- Reset ---------------------------------------
        public void Reset()
        {
            RxBindable.Invoke(()=>reset());
        }

        private void reset()
        {   
            for (int i=_MyList.Count(); --i>=0; )
               _MyList.RemoveAt(i);
        }
    }
}
