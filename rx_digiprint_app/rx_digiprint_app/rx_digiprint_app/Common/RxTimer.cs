using System;
using System.Collections.Generic;
using System.Threading;

namespace DigiPrint.Common
{

    public class RxTimer
    {
        private List<Action>     _Timer_List     = new List<Action>();
        private Timer            _Timer;

        public RxTimer(int interval)
        {
            _Timer = new Timer(new TimerCallback(OnTimer), _Timer_List, interval, interval);
        }

        ///--- Add_Action -----------------------------------------
        public void Add_Action(Action action)
        {
            _Timer_List.Add(action);
        }

        ///--- Remove_Action ----------------------
        public void Remove_Action(Action action)
        {
            _Timer_List.Remove(action);
        }

        //--- OnTimer ------------------------------------------
        void OnTimer(Object state)
        {
            foreach (Action action in _Timer_List) if (action!=null) action();
        }
    }

}
