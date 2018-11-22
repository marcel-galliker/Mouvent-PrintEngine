using RX_Common;
using RX_DigiPrint.Helpers;
using System;
using System.Collections.ObjectModel;
using System.Linq;

namespace RX_DigiPrint.Models
{
    public class NetworkList
    {
        static private ObservableCollection<NetworkItem> _Network;

        //--- constructor --------------------------------------------------------
        public NetworkList()
        {
            if (_Network == null)   _Network = new ObservableCollection<NetworkItem>();        
        }

        public ObservableCollection<NetworkItem> List
        {
            get { return _Network; }
        }

        //--- Property Enabled ---------------------------------------
        private bool _IsEnabled = false;
        public bool IsEnabled
        {
            get { return _IsEnabled; }
            set 
            { 
                if (value!=_IsEnabled)
                {
                    _IsEnabled = value;
                    foreach(NetworkItem item in RxGlobals.Network.List) item.DeviceNoEnabled=_IsEnabled;
                }
            }
        }
        
        //--- _compare_devices ----------------------------------------
        private int _compare_devices(NetworkItem item1, NetworkItem item2)
        {
            int diff=0;
            try
            {
                /*
                string[] parts1 = item1.IpAddr.Split('.');
                string[] parts2 = item2.IpAddr.Split('.');
                diff +=      Math.Sign(Convert.ToInt32(parts1[0])-Convert.ToInt32(parts2[0])); 
                diff +=   10*Math.Sign(Convert.ToInt32(parts1[1])-Convert.ToInt32(parts2[1]));  
                diff +=  100*Math.Sign(Convert.ToInt32(parts1[2])-Convert.ToInt32(parts2[2])); 
                diff += 1000*Math.Sign(Convert.ToInt32(parts1[3])-Convert.ToInt32(parts2[3]));  
                */
                int n1 = 1000*(int)item1.DeviceType + item1.DeviceNo;
                int n2 = 1000*(int)item2.DeviceType + item2.DeviceNo;
                diff = n1-n2;
            }
            catch
            {
            }
            return diff;
        }

        public void AddItem(NetworkItem item)
        {   
            Console.WriteLine("NetworkItem: {0} s# {1} d#{2}", item.DeviceType.ToString(), item.SerialNo, item.DeviceNo);
            int i;
            for (i = 0; i < _Network.Count(); i++ )
            {
                if (_Network[i].MacAddr==item.MacAddr)
                {
                    RxBindable.Invoke(() => _Network[i] = item); 
                    return;
                }
            }
            RxBindable.Invoke(() => 
            {
                for (int n=0; true; n++)
                {
                    if (n==_Network.Count()) {_Network.Add(item); return;};
                    if (_compare_devices(_Network[n], item)>0) {_Network.Insert(n, item); return;}
                };
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
            Console.WriteLine("Network RESET");   
            for (int i=_Network.Count(); --i>=0; )
               _Network.RemoveAt(i);
        }

        //--- GetWindowTitle ----------------------------
        public string GetWindowTitle(string title)
        {
            string[] list=title.Split(' ');
            for (int i=0; i<_Network.Count(); i++)
            {
                NetworkItem item=_Network[i] as NetworkItem;
                if (list[0].Equals(item.IpAddr))
                {
                    switch(item.DeviceType)
                    {
                        case Services.EDeviceType.dev_head:     return "HD "+item.DeviceNoList.GetDisplay(item.DeviceNo-1);
                        case Services.EDeviceType.dev_enc:      return "ENC";
                        case Services.EDeviceType.dev_fluid:    return "FL "+item.DeviceNo+1;
                        case Services.EDeviceType.dev_stepper:  return "ST "+item.DeviceNo+1;
                        case Services.EDeviceType.dev_main:     return "MAIN";
                    }
                }
            }

            return title;
        }

        //--- CreateLog -----------------------------------
        public void CreateLog(RxWorkBook wb)
        {
            int row = 0;
            int col;
            foreach(NetworkItem item in _Network)
            {
                row++;
                col=0;
                wb.setText(0, col, "Device Type");     wb.setText(row, col++, item.DeviceType);
                wb.setText(0, col, "No");              wb.setText(row, col++, item.DeviceNo);
                wb.setText(0, col, "SerialNo");        wb.setText(row, col++, item.SerialNo);
                wb.setText(0, col, "Mac Addredd");     wb.setText(row, col++, item.MacAddr);
                wb.setText(0, col, "IP  Address");     wb.setText(row, col++, item.IpAddr);
            }
            wb.HeaderRow(0);
        }
    }
}
