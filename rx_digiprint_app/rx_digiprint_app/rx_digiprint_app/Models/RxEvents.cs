using DigiPrint.Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xamarin.Forms;

namespace DigiPrint.Models
{
    public class RxEvents : RxBindable
    {
        public RxEvents()
        {
        }

        //--- constructor ----------------
        public RxEvents(RxBtDef.SErrorMsg msg)
        {
            Device  = msg.device;
            Message = msg.message;
            switch(msg.type)
            {
                case RxBtDef.ELogType.eErrLog:  Color = Color.White;    break;
                case RxBtDef.ELogType.eErrWarn: Color = Color.Yellow;   break;
                default:                        Color = Color.FromHex("#ff6f60");break;
            }
            AppGlobals.Printer.EventLevel = msg.type;
        }

        //--- Property Device ---------------------------------------
        private string _Device;
        public string Device
        {
            get { return _Device; }
            set { SetProperty(ref _Device, value); }
        }

        //--- Property Message ---------------------------------------
        private string _Message;
        public string Message
        {
            get { return _Message; }
            set { SetProperty(ref _Message, value); }
        }

        //--- Property Color ---------------------------------------
        private Color _Color;
        public Color Color
        {
            get { return _Color; }
            set { SetProperty(ref _Color, value); }
        }
    }
}
