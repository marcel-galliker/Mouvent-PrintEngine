using RX_Common;
using System;
using System.Collections.ObjectModel;
using System.Linq;

namespace RX_DigiPrint.Models
{
    public class MaterialList
    {
        private ObservableCollection<Material> _MyList  = new  ObservableCollection<Material>();

        public MaterialList()
        {
            AddItem(new Material(){Name="--- NEW ---"});
        }        
        
        public ObservableCollection<Material> List
        {
            get { return _MyList; }
        }
        
        //--- AddItem ----------------------------------------
        public void AddItem(Material item)
        {
            int idx=0;
            while (idx<_MyList.Count)
            {
                int d=string.Compare(_MyList[idx].Name, item.Name);
                if (d>0)
                {
                    RxBindable.Invoke(() => _MyList.Insert(idx, item));
                    return;
                }
                if (d==0)
                {
                    RxBindable.Invoke(() => _MyList[idx] = item);
                    return;
                }
                idx++;
            }
            RxBindable.Invoke(() => _MyList.Add(item));
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
            AddItem(new Material(){Name="--- NEW ---"});
        }

        //--- FindByName -------------------------------------
        public Material FindByName(string fileName)
        {
            foreach(Material material in _MyList)
            {
                if (material.Name.Equals(fileName)) return material;
            }
            return null;
        }

        //--- DeleteByName -------------------------------------
        public void DeleteByName(string fileName)
        {
            foreach(Material material in _MyList)
            {
                if (material.Name.Equals(fileName)) 
                {
                    RxBindable.Invoke(() => _MyList.Remove(material));
                    return;
                }
            }
        }


    }
}
