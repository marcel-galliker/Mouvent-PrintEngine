using System;
using System.Threading;
using System.Windows.Threading;

namespace RX_Common
{
    public class RxTimer
    {
        public delegate void TickDelegate(int no);
        public TickDelegate TimerFct = null;
        
        private Timer _Timer;
        private int _TickNo = 0;

        //--- Constructor ---------------------------------------
        public RxTimer(int interval)
        {
            if (_Timer==null) _Timer=new Timer(Tick, null, 0, interval); 
          //  _Timer.Tick     += new EventHandler(Tick);
          //  _Timer.Interval = new TimeSpan(0, 0, 0, 0, interval);
          //  _Timer.Start();
        }

        //--- Tick -----------------------------------------------------------------------
//        private void Tick(object o, EventArgs sender)
        private void Tick(object o)
        {
             if (TimerFct!=null) RxBindable.Invoke(()=>TimerFct(_TickNo));
             _TickNo++;
        }

    }
}
