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
    public class ClusterStatList
    {
        static private ObservableCollection<ClusterStat> _MyList = new ObservableCollection<ClusterStat>();

        //--- constructor -----------------------------
        public ClusterStatList()
        {
        }

        //--- Property List ----------------------
        public ObservableCollection<ClusterStat> List
        {
            get { return _MyList; }
        }

        //--- SetItemCount -----------------
        public void SetItemCount(int cnt)
        {
            while (_MyList.Count < cnt) _MyList.Add(new ClusterStat());
        }

        public int Number
        {
            get { return _MyList.Count; }
        }

        //--- SetItem -----------------------------
        public void SetItem(TcpIp.SHeadBoardStat item)
        {
            RxBindable.Invoke(() => 
            {
                int no = (int) item.boardNo;
                SetItemCount(no + 1);
                _MyList[no].SetItem(item);
            }
            );
        }

        private void reset()
        {   
            for (int i=_MyList.Count(); --i>=0; )
               _MyList.RemoveAt(i);
        }
    }
}
