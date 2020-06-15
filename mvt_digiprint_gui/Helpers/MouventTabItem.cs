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
    public class MouventTabItem : TabItemEx
    {
        //--- constructor
        public MouventTabItem()
        {
        }

        //--- Property Changed ---------------------------------------
        public bool Changed
        {
            get { return (bool)GetValue(ChangedProperty); }
            set 
            { 
                SetValue(ChangedProperty, value); 
            }
        }

        public static readonly DependencyProperty ChangedProperty =
            DependencyProperty.Register("Changed", typeof(bool), typeof(MouventTabItem), new PropertyMetadata(false));
    }
}
