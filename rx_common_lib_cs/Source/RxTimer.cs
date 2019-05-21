using System;
using System.Windows.Threading;

namespace RX_Common
{
    public class RxTimer
    {
        public delegate void TickDelegate(int no);
        public TickDelegate TimerFct = null;
        
        private DispatcherTimer _Timer=new DispatcherTimer();
        private int _TickNo = 0;

        //--- Constructor ---------------------------------------
        public RxTimer(int interval)
        {
            _Timer.Tick     += new EventHandler(Tick);
            _Timer.Interval = new TimeSpan(0, 0, 0, 0, interval);
            _Timer.Start();
        }

        //--- Start --------------------------------------
        public void Start()
        {
            _Timer.Start();
        }

        //--- Stop ----------------------------------------
        public void Stop()
        {
            _Timer.Stop();
        }

        //--- Tick -----------------------------------------------------------------------
        private void Tick(object o, EventArgs sender)
        {
             if (TimerFct!=null) RxBindable.Invoke(()=>TimerFct(_TickNo));
             _TickNo++;
        }

    }
}
