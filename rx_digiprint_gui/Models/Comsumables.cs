using RX_Common;
using System;
using System.Collections.Generic;

namespace RX_DigiPrint.Models
{
    public class Comsumables : RxBindable
    {
        //--- Constuctor -------------------
        public Comsumables()
        {
        }

        //--- Property Unwinder ---------------------------------------
        private UInt32 _Unwinder;
        public UInt32 Unwinder
        {
            get { return _Unwinder; }
            set { SetProperty(ref _Unwinder, value); }
        }

        //--- Property Rewinder ---------------------------------------
        private UInt32 _Rewinder;
        public UInt32 Rewinder
        {
            get { return _Rewinder; }
            set { SetProperty(ref _Rewinder, value); }
        }

        //--- Property Ink ---------------------------------------
        private List<UInt32> _Ink;
        public List<UInt32> Ink
        {
            get { return _Ink; }
            set { SetProperty(ref _Ink, value); }
        }
        
    }
}
