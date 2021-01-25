using System;
using System.Threading;
using System.Windows.Threading;

namespace RX_Common
{
    public class RxTimer
    {
        public delegate void TickDelegate(int no);
        public TickDelegate TimerFct = null;
        
        private Timer   _Timer;
        private int     _TickNo = 0;
        static private long    _Ticks0 = DateTime.Now.Ticks;

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

        public string Ticks()
		{
            long ticks=(DateTime.Now.Ticks-_Ticks0) / 10000;
            return string.Format("{0:N0}", ticks);
		}

        static public int GetTicks()
		{
            long ticks=(DateTime.Now.Ticks-_Ticks0) / 10000;
            return (int)ticks;
		}

    }
}
