using DigiPrint.Common;
using RX_DigiPrint.Services;
using System;
using System.Diagnostics;
using Xamarin.Forms;
using static RX_DigiPrint.Services.RxBtDef;

namespace DigiPrint.Models
{
    public class Printer : RxBindable
    {
        private string _Name;

        public Printer()
        {
            EventLevel = RxBtDef.ELogType.eErrUndef;
        }

        //--- Update -----------------------------------
        public void Update(SReplyStateMsg msg)
        {
            State           = msg.state;
            ColorCnt        = msg.colorCnt;
            HeadsPerColor   = msg.headsPerColor;
            Reverse         = msg.reverse!=0;
        }        

        //--- Property MyProperty ---------------------------------------
        private int _MyProperty;
        public int MyProperty
        {
            get { return _MyProperty; }
            set { SetProperty(ref _MyProperty, value); }
        }

        //--- Property Name ----------------------------------------
        public string Name
        {
            get { return _Name; }
            set { SetProperty(ref _Name, value);}
        }

        //--- Property State ---------------------------------------
        private RxBtDef.EBTState _State;
        public RxBtDef.EBTState State
        {
            get { return _State; }
            set { SetProperty(ref _State, value); }
        }

        //--- Property EventLevel ---------------------------------------
        private RxBtDef.ELogType _EventLevel;
        public Action OnEventIconChanged;

        public RxBtDef.ELogType EventLevel
        {
            get { return _EventLevel; }
            set 
            { 
                if (value > _EventLevel || value==RxBtDef.ELogType.eErrUndef)
                {
                    SetProperty(ref _EventLevel, value);
                    if (value>RxBtDef.ELogType.eErrWarn)
                    {
                        EventBackground = Color.Red;
                        EventTextColor  = Color.Black;
                        EventIcon       = "error.png";
                    }
                    else if (value==RxBtDef.ELogType.eErrWarn)
                    {
                        EventBackground = Color.Yellow;
                        EventTextColor  = Color.Black;
                        EventIcon       = "warning.png";
                    }
                    else
                    {
                        EventBackground = Color.WhiteSmoke;
                        EventTextColor  = Color.Black;
                        EventIcon       = "info.png";
                    }
                }
            }
        }

        //--- Property EventBackground ---------------------------------------
        private Color _EventBackground;
        public Color EventBackground
        {
            get { return _EventBackground; }
            set { SetProperty(ref _EventBackground, value);}
        }

        //--- Property EventTextColor ---------------------------------------
        private Color _EventTextColor;
        public Color EventTextColor
        {
            get { return _EventTextColor; }
            set { SetProperty(ref _EventTextColor, value); }
        }

        //--- Property EventIcon ---------------------------------------
        private string _EventIcon="ok.png";
        public string EventIcon
        {
            get { return _EventIcon; }
            set { if(SetProperty(ref _EventIcon, value) && (OnEventIconChanged!=null)) OnEventIconChanged(); }
        }

        //--- Property ColorCnt ---------------------------------------
        private int _ColorCnt;
        public int ColorCnt
        {
            get { return _ColorCnt; }
            set { SetProperty(ref _ColorCnt, value); }
        }
        //--- Property HeadsPerColor ---------------------------------------
        private int _HeadsPerColor;
        public int HeadsPerColor
        {
            get { return _HeadsPerColor; }
            set 
            {
                int i, headCnt = ColorCnt*value;
                if (headCnt!=AppGlobals.PrintHeads.Count)
                {
                    if (AppGlobals.PrintHeads.Count>0)
                        AppGlobals.PrintHeads.Clear();
                    for (i=0; i< headCnt; i++) AppGlobals.PrintHeads.Add(new PrintHead(i));
                    SetProperty(ref _HeadsPerColor, value);
                }
            }
        }
        //--- Property Reverse ---------------------------------------
        private bool _Reverse;
        public bool Reverse
        {
            get { return _Reverse; }
            set { SetProperty(ref _Reverse, value); }
        }
    }
}
