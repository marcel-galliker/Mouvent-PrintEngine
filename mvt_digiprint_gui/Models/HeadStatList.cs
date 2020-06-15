using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
    public class HeadStatList
    {
        static private ObservableCollection<HeadStat> _MyList = new ObservableCollection<HeadStat>();
        static Boolean[] _BoardConnected = new Boolean[128];

        //--- constructor -----------------------------
        public HeadStatList()
        {
            SetItemCount(TcpIp.InkSupplyCnt);
        }

        //--- Property List ----------------------
        public ObservableCollection<HeadStat> List
        {
            get { return _MyList; }
        }

        //--- SetItemCount -----------------
        public void SetItemCount(int cnt)
        {
            while (_MyList.Count<cnt) _MyList.Add(new HeadStat());                
        }

        //--- SetItem -----------------------------
        public void SetItem(int no, TcpIp.SHeadStat item, Int32 tempFpga, Int32 flow)
        {
            RxBindable.Invoke(() => 
            {            
                SetItemCount(no+1);
                _MyList[no].SetItem(no, item, tempFpga, flow);
                _MyList[no].Connected = _BoardConnected[no/4];
            }
            );
        }

        //--- SetConnected -----------------------------
        public void SetConnected(int boardNo, Boolean connected)
        {
            if (boardNo < _BoardConnected.Length) _BoardConnected[boardNo]=connected;
            if (_MyList==null) return;
            RxBindable.Invoke(() => 
            {          
                foreach(HeadStat item in _MyList)
                {
                    if ((item.HeadNo/4) == boardNo) item.Connected = connected;
                }
            }
            );
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
