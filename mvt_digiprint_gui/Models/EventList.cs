using RX_Common;
using RX_DigiPrint.Services;
using System.Collections.ObjectModel;
using System.Linq;

namespace RX_DigiPrint.Models
{
    public class EventList
    {
        static private ObservableCollection<LogItem> _List;
        private bool _alternate=false;
        
        public EventList()
        {
            if (_List == null)
            {
                _List = new ObservableCollection<LogItem>();
                /*
                _List.Add(new LogItem() { Error = 1, Message="LOG 1"});
                _List.Add(new LogItem() { Error = 2, Message="LOG 2"});
                _List.Add(new LogItem() { Error = 3, Message="LOG 3"});
                _List.Add(new LogItem() { Error = 4, Message="LOG 4"});

                _List.Add(new LogItem() { Error = 101, Message="WARN 1", LogType = ELogType.eErrWarn});
                _List.Add(new LogItem() { Error = 102, Message="WARN 2", LogType = ELogType.eErrWarn});
                _List.Add(new LogItem() { Error = 103, Message="WARN 3", LogType = ELogType.eErrWarn});
                _List.Add(new LogItem() { Error = 104, Message="WARN 4", LogType = ELogType.eErrWarn});

                _List.Add(new LogItem() { Error = 1001, Message="ERR 1", LogType = ELogType.eErrCont});
                _List.Add(new LogItem() { Error = 1002, Message="ERR 2", LogType = ELogType.eErrCont});
                _List.Add(new LogItem() { Error = 1003, Message="ERR 3", LogType = ELogType.eErrStop});
                _List.Add(new LogItem() { Error = 1004, Message="ERR 4", LogType = ELogType.eErrAbort});
                */
            }
        }

        public ObservableCollection<LogItem> List
        {
            get { return _List; }
            set { }
        }
        
        public void AddItem(LogItem item)
        {
            RxBindable.Invoke(() => 
            {
                item.Alternate = _alternate;
                _alternate = !_alternate;
                _List.Insert(0, item);
                while (_List.Count>100) _List.RemoveAt(100);
            });
        }

        //--- Reset ---------------------------------------
        public void Reset()
        {
            RxBindable.Invoke(()=>_List.Clear());
            _alternate=false;
        }

    }
}
