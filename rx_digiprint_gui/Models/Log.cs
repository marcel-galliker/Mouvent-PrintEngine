using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Services;
using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Threading;
using System.Windows.Media;

namespace RX_DigiPrint.Models
{
    public class LogList : ObservableCollection<LogItem>
    {
        private static ObservableCollection<LogItem> _List = new ObservableCollection<LogItem>();
        private RxWorkBook  _WB;
        private int         _WB_count=100;
        private int         _WB_row;
        private bool        _alternate=false;
        
        //--- Property List ----------------------------------------------
        public ObservableCollection<LogItem> List
        {
            get { return _List; }
        }

        //--- ItemCount -----------------------------------------------------
        private static Int32 _ItemCount=0;
        public Int32 ItemCount
        {
            get { return _ItemCount; }
            set 
            { 
                Reset();
                _ItemCount = value; 
            }
        }

        //--- Property Pos ---------------------------------------
        private Int32 _Pos;
        public Int32 Pos
        {
            get { return _Pos; }
//            set { SetProperty(ref _Pos, value); }
            set 
            { 
                _Pos = value;
            }
        }
        
        //--- AddItem ------------------------------------------------------
        public void AddItem(LogItem item)
        {
            if (_WB!=null)
            {
                int col=0;
                _WB.setText(0, col, "Time");     _WB.setText(_WB_row, col++, item.TimeStr);
                _WB.setText(0, col, "Type");     _WB.setText(_WB_row, col++, item.LogTypeStr);
                _WB.setText(0, col, "No");       _WB.setText(_WB_row, col++, item.Error);
                _WB.setText(0, col, "Message");  _WB.setText(_WB_row, col++, item.Message);
                switch (item.LogType)
                {
                    case ELogType.eErrLog:   break;
                    case ELogType.eErrWarn:  _WB.SetRowColor(_WB_row, Colors.Black, Colors.Orange); break;
                    default:                 _WB.SetRowColor(_WB_row, Colors.Black, Colors.Red); break;
                }
               
                if (_WB_row==0) _WB.HeaderRow(0);
                _WB_row++;            
                if (_WB_row==_WB_count) 
                {
                    _WB.SizeColumns();
                    _WB.write(_WB.FileName);
                    _WB=null;      
                }
            }
            else RxBindable.Invoke(() => 
            {
                item.Alternate = _alternate;
                _alternate = !_alternate;
            //    _List.Insert(0, item);
                _List.Add(item);
            });
        }

        //--- Reset --------------------------------------------------------
        public void Reset()
        {
            if(_WB==null) RxBindable.Invoke(()=>reset());
        }

        private void reset()
        {   
            for (int i=_List.Count(); --i>=0; )
               _List.RemoveAt(i);
        }

        //--- CreateLog ----------------------------------------
        public void CreateLog(RxWorkBook wb)
        {
            _WB = wb;
            _WB_row = 0;
            TcpIp.SLogReqMsg msg = new TcpIp.SLogReqMsg();
            msg.first = 0;
            msg.count = _WB_count;          
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_REQ_LOG, ref msg);
        }

    }
}
