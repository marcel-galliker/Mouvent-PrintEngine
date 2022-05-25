using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;

namespace RX_DigiPrint.Models
{
    public class ClusterNo : RxBindable
    {

        public ClusterNo()
        {
        }

        //--- Property C1No ---------------------------------------
        private double _C1No = 0;
        public double C1No
        {
            get { return _C1No; }
            set
            {
                SetProperty(ref _C1No, value);
            }
        }

        //--- Property C2No ---------------------------------------
        private double _C2No = 0;
        public double C2No
        {
            get { return _C2No; }
            set
            {
                SetProperty(ref _C2No, value);
            }
        }

        //--- Property C3No ---------------------------------------
        private double _C3No = 0;
        public double C3No
        {
            get { return _C3No; }
            set
            {
                SetProperty(ref _C3No, value);
            }
        }

        //--- Property C4No ---------------------------------------
        private double _C4No = 0;
        public double C4No
        {
            get { return _C4No; }
            set
            {
                SetProperty(ref _C4No, value);
            }
        }

        public void Set_ClusterNo(int clusterNo)
        {

            TcpIp.SValue value = new TcpIp.SValue();
            value.no = clusterNo;
            if (clusterNo == 1) value.value = (Int32)C1No;
            if (clusterNo == 2) value.value = (Int32)C2No;
            if (clusterNo == 3) value.value = (Int32)C3No;
            if (clusterNo == 4) value.value = (Int32)C4No;

            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_CHANGE_CLUSTER_NO, ref value);
        }
    }
}
