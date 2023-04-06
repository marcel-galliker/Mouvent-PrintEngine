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
        static Boolean[] _BoardConnected 	= new Boolean[128];
		static Boolean[] _MeniscusDisabled  = new Boolean[128];
		static UInt32[]  _ClusterErr        = new UInt32[12];
        static UInt32[]  _ClusterNo 		= new UInt32[12];
        static Int32[] 	 _Cooler_Pressure 	= new Int32[12];
        static UInt32[]  _Cooler_Temp 		= new UInt32[12];
        static Int32[]   _FP_Voltage 		= new Int32[12];

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
                _MyList[no].Meniscus_Disabled = _MeniscusDisabled[no / 4];
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
                    if (item.HeadNo>=0 && (item.HeadNo/4) == boardNo) item.Connected = connected;
                }
            }
            );
        }

		//--- Meniscus_disabled ------------------------
		public void Meniscus_disabled(int boardNo, Boolean meniscus_disabled)
        {
            if (boardNo < _MeniscusDisabled.Length) _MeniscusDisabled[boardNo] = meniscus_disabled;
            if (_MyList == null) return;
            RxBindable.Invoke(() =>
            {
                foreach (HeadStat item in _MyList)
                {
                    if ((item.HeadNo / 4) == boardNo) item.Meniscus_Disabled = meniscus_disabled;
                }
            }
            );
        }

		//--- Cooler_Pressure --------------------------------------
        public void Cooler_Pressure(int boardNo, Int32 cooler_pressure)
        {
            if (boardNo < _Cooler_Pressure.Length) _Cooler_Pressure[boardNo] = cooler_pressure;
            if (_MyList == null) return;
            RxBindable.Invoke(() =>
            {
                foreach (HeadStat item in _MyList)
                {
                    if ((item.HeadNo / 4) == boardNo) item.Cooler_Pressure = cooler_pressure;
                }
            }
            );
        }

		//--- Cooler_Temp ------------------------------------------
        public void Cooler_Temp(int boardNo, UInt32 cooler_temp)
        {
            if (boardNo < _Cooler_Temp.Length) _Cooler_Temp[boardNo] = cooler_temp;
            if (_MyList == null) return;
            RxBindable.Invoke(() =>
            {
                foreach (HeadStat item in _MyList)
                {
                    if ((item.HeadNo / 4) == boardNo) item.Cooler_Temp = cooler_temp;
                }
            }
            );
        }

		//--- FP_Voltage ---------------------------
		public void FP_Voltage(int boardNo, Int32 voltage)
        {
            if (boardNo < _FP_Voltage.Length) _FP_Voltage[boardNo] = voltage;
            if (_MyList == null) return;
            RxBindable.Invoke(() =>
            {
                foreach (HeadStat item in _MyList)
                {
                    if ((item.HeadNo / 4) == boardNo) item.FP_Voltage = voltage;
                }
            }
            );
        }

        //--- ClusterNo -----------------------------------------------------------------
        public void ClusterNo(int boardNo, UInt32 clusterNo)
        {
            if (boardNo < _ClusterNo.Length) _ClusterNo[boardNo] = clusterNo;
            if (_MyList == null) return;
            RxBindable.Invoke(() =>
            {
                foreach (HeadStat item in _MyList)
                {
                    if ((item.HeadNo / 4) == boardNo) item.ClusterNo = clusterNo;
                }
            }
            );
        }

		//--- SetClusterErr ----------------
        public void SetClusterErr(int cluster, UInt32 err)
        {
            if (cluster<_ClusterErr.Length) _ClusterErr[cluster]=err;
        }

		//--- GetClusterErr -----------------------------
		public UInt32 GetClusterErr(int cluster)
        {
            if (cluster<_ClusterErr.Length) return _ClusterErr[cluster];
            else return 0;
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
