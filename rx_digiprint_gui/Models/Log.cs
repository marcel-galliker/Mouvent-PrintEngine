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
            set { _Pos = value;}
        }
        
        //--- AddItem ------------------------------------------------------
        public void AddItem(LogItem item)
        {
            RxBindable.Invoke(() => 
            {
                item.Alternate = _alternate;
                _alternate = !_alternate;
                _List.Add(item);
            });
        }

        //--- Reset --------------------------------------------------------
        public void Reset()
        {
            RxBindable.Invoke(()=>reset());
        }

        private void reset()
        {   
            for (int i=_List.Count(); --i>=0; )
               _List.RemoveAt(i);
        }
    }
}
