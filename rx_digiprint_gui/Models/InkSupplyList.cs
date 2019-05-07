using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.ObjectModel;
using System.Linq;

namespace RX_DigiPrint.Models
{
    public class InkSupplyList
    {
        static private ObservableCollection<InkSupply> _MyList;

        public InkSupplyList()
        {
            if (_MyList==null)
            {
                _MyList  = new  ObservableCollection<InkSupply>();
                for (int i=0; i<TcpIp.InkSupplyCnt+2; i++)
                {
                    _MyList.Add(new InkSupply());
                    _MyList[i].No=i+1;
                }
            }
        }

        //--- Property List ----------------------
        public ObservableCollection<InkSupply> List
        {
            get { return _MyList; }
        }
        
        //--- Add Item -----------------------------
        public void AddItem(InkSupply item)
        {
            RxBindable.Invoke(() => _MyList.Add(item));
        }

        //--- SetConnected -----------------------------
        public void SetConnected(int boardNo, Boolean connected)
        {
            if (_MyList==null) return;
            RxBindable.Invoke(() => 
            {          
                foreach(InkSupply item in _MyList)
                {
                    if (((item.No-1)/4) == (boardNo-1))
                    {
                        item.Connected = connected;
                    }
                }
            }
            );
        }

        //--- Update ------------------------------------
        public void Update(TcpIp.SInkSupplyStatMsg msg)
        {
            int i;
            for (i=0; i<msg.status.Count() && i<_MyList.Count(); i++)
            {   
                _MyList[i].SetStatus(i, msg.status[i]);
            }
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
