using System;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Helpers
{
    class RxProgressBar : ProgressBar
    {
        //--- Property ValueStr ---------------------------------------
        public String ValueStr
        {
            get { return (String)this.GetValue(ValueStrProperty); }
            set { this.SetValue(ValueStrProperty, value); }
        }

        //--- ValueStrChanged -----------------------------------------------
        private static void ValueStrChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            RxProgressBar p = d as RxProgressBar;
            p.FontSize=12;
            try
            {
                double val = Convert.ToDouble(e.NewValue);
                if (p != null && p.Minimum <= val && val <= p.MaxHeight)
                {
                    p.Value = val;
                }
            }
            catch (Exception)
            {
                p.Value = 0;
            }
        }

        public static readonly DependencyProperty ValueStrProperty = DependencyProperty.Register("ValueStr", typeof(String), typeof(RxProgressBar), new PropertyMetadata("", ValueStrChanged) );
    }
}
