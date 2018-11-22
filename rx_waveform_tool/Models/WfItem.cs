using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using RxWfTool.Helpers;

namespace RxWfTool.Models 
{
    public class WfItem : NotificationObject
    {
        public delegate void LenChanged(int no, int delta);

        public static LenChanged OnLenChanged;
        
        public static int MaxVolt = 30;
//      public static int MaxPos  = 4096;
        public static int MaxPos  = 2000;
        public static int FirstPulsePos = 144;
        public static double Interval = 12.5; // intervall in ns

        //--- Property No ------------------------------
        private int _no;
        public int No
        {
            get { return _no; }
            set { SetProperty(ref _no, value); }
        }
        
        //--- Property Pisition ---------------------------
        private int _pos;
        public int Position
        {
            get { return _pos; }
            set 
            { 
                int pos=value;
                if (pos<0) pos=0;
                if (pos>MaxPos) pos=MaxPos;
                if (SetProperty(ref _pos, pos))
                {
                    Time  = pos/80.0;
                    if (ListChanged != null) ListChanged(this, new EventArgs());
                }
            }
        }

        //--- Property Voltage ---------------------------
        private int _voltage;
        public int Voltage
        {
            get { return _voltage; }
            set 
            { 
                int volt=value;
                if (volt<0) volt=0;
                if (volt>MaxVolt) volt=MaxVolt;
                if(SetProperty(ref  _voltage, volt))
                {
                    if (ListChanged != null) ListChanged(this, new EventArgs());
                }
            }
        }

        //--- Property Time ---------------------------------------
        private double _Time;
        public double Time
        {
            get { return _Time; }
            set { SetProperty(ref _Time, value); }
        }

        //--- Property Delta ---------------------------------------
        private double _Delta;
        public double Delta
        {
            get { return _Delta; }
            set { SetProperty(ref _Delta, value); }
        }

        //--- Property Len ---------------------------------------
        private int? _Len;
        public int? Len
        {
            get { return _Len; }
            set 
            { 
                if (_Len==null || value==null)
                {
                    SetProperty(ref _Len, value);
                }
                else 
                {
                    int delta=(int)value-(int)_Len;
                    if (SetProperty(ref _Len, value) && OnLenChanged != null)
                    {
                        if (_no>0) OnLenChanged(_no-1, delta);
                    }
               }
            }
        }
        
        //--- Event ListChanged ----------------------------
        public event EventHandler<EventArgs> ListChanged;
    }
}
