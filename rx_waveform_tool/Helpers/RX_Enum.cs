using System;
using System.Collections;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using RX_GUI.Helpers;
using RxWfTool.Helpers;

namespace RX_GUI.Helpers
{
    public class RX_Enum<type> : NotificationObject
    {
        public RX_Enum()
        {
        }

        public RX_Enum(type value, string display)
        {
            Value = value;
            Display = display;
        }

        private type _Value;
        public type Value
        {
            get { return _Value; }
            set { SetProperty<type>(ref _Value, value); }
        }

        private string _Display;
        public string Display
        {
            get { return _Display; }
            set { SetProperty<string>(ref _Display, value); }
        }
    }
}
