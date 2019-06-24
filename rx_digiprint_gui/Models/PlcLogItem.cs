using RX_Common;
using RxRexrothGui.Services;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RxRexrothGui.Models
{
    public class CPlcLogItem : RxBindable
    {
        //--- Constructor ----------------------
        public CPlcLogItem(RxRexroth.SPlcLogItem item)
        {
            No     = item.no+1;
            State  = item.state; 
            ErrNo  = item.errNo;
            Time   = item.date;
            Text   = item.text;
        }

        //--- Property No ---------------------------------------
        private int _No;
        public int No
        {
            get { return _No; }
            set { SetProperty(ref _No, value); }
        }

        //--- Property State ---------------------------------------
        private RxRexroth.EnPlcLogState _State;
        public RxRexroth.EnPlcLogState State
        {
            get { return _State; }
            set { SetProperty(ref _State, value); }
        }
        
        //--- Property ErrNo ---------------------------------------
        private UInt32 _ErrNo;
        public UInt32 ErrNo
        {
            get { return _ErrNo; }
            set { SetProperty(ref _ErrNo, value); }
        }        

        //--- Property Time ---------------------------------------
        private string _Time;
        public string Time
        {
            get { return _Time; }
            set { SetProperty(ref _Time, value); }
        }

        //--- Property Text ---------------------------------------
        private string _Text;
        public string Text
        {
            get { return _Text; }
            set { SetProperty(ref _Text, value); }
        }

    }

    public class CPlcLogCollection : ObservableCollection<CPlcLogItem> { }
}
