using RX_Common;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;

namespace rx_rip_gui.Models
{  
    public class PrEnvList : RxBindable
    {
        static public string DoNotRip = "--- DO NOT RIP ---";

        //--- PrintEnvList ----------------------------------
        private ObservableCollection<PrEnv> _List = new ObservableCollection<PrEnv>();
        public ObservableCollection<PrEnv> List
        {
            get { return _List;  }
        }

        //--- AddItem ------------------------------------
        public void AddItem(PrEnv item)
        {
            if (!_List.Contains(item))
                RxBindable.Invoke(()=>_List.Add(item));       
        }

        //--- Reset -----------------------------------------
        public void Reset()
        {   
            RxBindable.Invoke(()=>
            {
                for (int i=_List.Count(); --i>=0; )
                   _List.RemoveAt(i);
            });
        }

   }

}
