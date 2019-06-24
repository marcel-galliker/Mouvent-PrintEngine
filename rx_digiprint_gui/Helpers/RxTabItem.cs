using System.Windows;
using System.Windows.Controls;
using Infragistics.Windows.Controls;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using RX_DigiPrint.Services;
using System.Windows.Media;

namespace RX_DigiPrint.Helpers
{
    public class RxTabItem : TabItemEx
    {
        //--- constructor
        public RxTabItem()
        {
        }

        //--- Property Changed ---------------------------------------
        public bool Changed
        {
            get { return (bool)GetValue(ChangedProperty); }
            set 
            { 
                /*
                if (value) 
                {
                    if (!Header.ToString().EndsWith("*")) this.Header = this.Header+"*";                    
                }
                else if (Header.ToString().EndsWith("*")) this.Header = this.Header.ToString().Remove(this.Header.ToString().Length-1);
                */
                SetValue(ChangedProperty, value); 
            }
        }

        public static readonly DependencyProperty ChangedProperty =
            DependencyProperty.Register("Changed", typeof(bool), typeof(RxTabItem), new PropertyMetadata(false));
    }
}
