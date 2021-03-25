using RX_Common;
using RX_DigiPrint.Converters;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;

namespace RX_DigiPrint.Models
{
    public class ClusterStat : RxBindable
    {
        //--- Constructor ----------------------------------------
        public ClusterStat()
        {
        }
        //--- Property board no---------------------------------------
        private UInt32 _no;
        public UInt32 BoardNo
        {
            get { return _no; }
            set { SetProperty(ref _no, value); }
        }

        //--- Property SerialNo ---------------------------------------
        private ulong _SerialNo;
        public ulong SerialNo
        {
            get { return _SerialNo; }
            private set { SetProperty(ref _SerialNo, value); }
        }

        //--- Property clusterTime ---------------------------------------
        private UInt32 _clusterTime;
        public UInt32 ClusterTime
        {
            get { return _clusterTime; }
            set { SetProperty(ref _clusterTime, value); }
        }

        //--- SetItem ----------------------------------------------
        public void SetItem(TcpIp.SHeadBoardStat item)
        {
            BoardNo = item.boardNo + 1;
            if ((item.info & 1) != 0)
            {
                SerialNo = item.serialNo;
                UInt64 printingSecondsSum = 0;
                for (int i = 0; i < item.head.Length; i++) printingSecondsSum += item.head[i].printingSeconds;
                ClusterTime = (UInt32)(printingSecondsSum / (UInt64)item.head.Length);
            }
            else
            {
                ClusterTime = TcpIp.INVALID_VALUE;
                SerialNo = TcpIp.INVALID_VALUE;
            }
        }
    }
}
