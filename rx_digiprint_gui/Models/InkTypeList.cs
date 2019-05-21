using RX_Common;
using System;
using System.Collections.ObjectModel;
using System.Linq;

namespace RX_DigiPrint.Models
{
    public class InkTypeList
    {
        static private ObservableCollection<InkType> _MyList  = new  ObservableCollection<InkType>();
                
        public ObservableCollection<InkType> List
        {
            get { return _MyList; }
        }
        
        //--- AddItem ----------------------------------------
        public void AddItem(InkType item)
        {
            RxGlobals.InkFamilies.AddItem(item.Family);
            int idx=0;
       //   while (idx<_MyList.Count && string.Compare(_MyList[idx].Description, item.Description)<0) 
            while (idx<_MyList.Count && string.Compare(_MyList[idx].Name, item.Name)<0) 
                idx++;
            RxBindable.Invoke(() => _MyList.Insert(idx, item));
        }

        //--- Reset ---------------------------------------
        public void Reset()
        {
            RxBindable.Invoke(()=>reset());
        }

        private void reset()
        {   
            RxGlobals.InkFamilies.Reset();
            for (int i=_MyList.Count(); --i>=0; )
               _MyList.RemoveAt(i);
        }

        //--- FindByFileName -------------------------------------
        public InkType FindByFileName(string fileName)
        {
            foreach(InkType i in _MyList)
            {
                if (i.FileName.Equals(fileName)) return i;
            }
            return null;
        }

    }
}
