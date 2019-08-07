using RX_Common;
using RX_DigiPrint.Services;
using System.Collections.ObjectModel;
using System.Linq;

namespace RX_DigiPrint.Models
{
    public class PrintQueue
    {
        static private ObservableCollection<PrintQueueItem> _Queue;
        static private ObservableCollection<PrintQueueItem> _Printed;
     
        //--- constructor ---------------------------------------------------------------
        public PrintQueue()
        {
            if (_Queue   == null) _Queue   = new ObservableCollection<PrintQueueItem>();
            if (_Printed == null) _Printed = new ObservableCollection<PrintQueueItem>();
        }

        //--- Property Queue --------------------------------------------------------------
        public ObservableCollection<PrintQueueItem> Queue
        {
            get { return _Queue; }
        }

        //--- Property Printed ----------------------------------------------------------
        public ObservableCollection<PrintQueueItem> Printed
        {
            get { return _Printed; }
        }

        //--- Reset ---------------------------------------
        public void Reset()
        {
            RxBindable.Invoke(()=>reset());
        }

        //--- internal  reset ---------------------------------------
        private void reset()
        {   
            _Queue.Clear();
            _Printed.Clear();
        }

        //--- AddItem ---------------------------------------------
        public void AddItem(PrintQueueItem item, bool top)
        {       
            if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_test_table 
            ||  RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_test_slide_only
            || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_slide)
            {
                _addToList      (_Queue,   item, top);
                return;
            }     
            if (item.State==EPQState.queued)   
            {
                if (_removeFromList (_Printed, item)) RxBindable.Invoke(() => _Queue.Add(item));
                else _addToList(_Queue,   item, top);
                if (_Queue.Count()==1) item.SendBtProdState();
            }
            else
            { 
                _removeFromList (_Queue,   item);
                _updateList     (_Printed, item);
                item.SendBtProdState();
            }
        }

        //--- RemoveItem ------------------------------------------------------------------------
        public void RemoveItem(PrintQueueItem item)
        {
            RxBindable.Invoke(() => 
            {
                if (item==null) _Queue.Clear();
                else _removeFromList(_Queue, item);
                if (_Queue.Count()==0) new PrintQueueItem().SendBtProdState(); 
            });
        }

        //--- UpItem ------------------------------------------------------------------------
        public void UpItem(PrintQueueItem item)
        {
            for (int i=0; i<_Queue.Count(); i++)
            {
                if (_Queue[i].ID==item.ID)
                {
                    RxBindable.Invoke(()=> move(i, -1));
                    return;
                }
            }
        }

        //--- DownItem ------------------------------------------------------------------------
        public void DownItem(PrintQueueItem item)
        {

            for (int i=0; i<_Queue.Count(); i++)
            {
                if (_Queue[i].ID==item.ID)
                {
                    RxBindable.Invoke(()=> move(i, 1));
                    return;
                }
            }
        }

        //--- move  --------------------------------------------------------------------------
        private void move(int i, int d)
        {
            if (i+d>=0 && i+d<_Queue.Count)
            { 
                PrintQueueItem x = _Queue[i+d];
                _Queue[i+d]=_Queue[i];
                _Queue[i] = x;
            }
        }

        //--- addToList -----------------------------------------------------------------------
        private void _addToList(ObservableCollection<PrintQueueItem> list, PrintQueueItem item, bool top)
        {
            for (int i=0; i<list.Count(); i++)
            {
                if (list[i].ID==item.ID)
                {
                    RxBindable.Invoke(()=>list[i]=item);
                    return;
                }
            }
            if (top) RxBindable.Invoke(() => list.Add(item));
            else     RxBindable.Invoke(() => list.Insert(0, item));
        }

        //--- removeFromList -----------------------------------------------------------------------
        private bool _removeFromList(ObservableCollection<PrintQueueItem> list, PrintQueueItem item)
        {
            for (int i=0; i<list.Count(); i++)
            {
                if (list[i].ID==item.ID && list[i].State<=EPQState.printed)
                {
                    RxBindable.Invoke(()=>list.RemoveAt(i));
                    return true;
                }
            }
            return false;
        }

        //--- _updateList -----------------------------------------------------------------------
        private void _updateList(ObservableCollection<PrintQueueItem> list, PrintQueueItem item)
        {
            for (int i = 0; i < list.Count(); i++)
            {
                if (list[i].ID == item.ID)
                {
                    RxBindable.Invoke(() => list[i].Update(item));
                    return;
                }
            }
            RxBindable.Invoke(() => list.Insert(0, item));
        }
    }

}
