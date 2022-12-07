using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Drawing;
using System.Windows;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    //--- HeadVal_Converter ----------------------------------------------------------------------
    public class HeadValRange_Converter : IValueConverter
    {

        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if (parameter!=null)
            {
                string[] par=(parameter as string).Split(';');
                int from =System.Convert.ToInt32(par[0]);
                int to   =System.Convert.ToInt32(par[0]);
                int val  = (int)value;
                if (val>=from && val<=to) return Visibility.Collapsed;
            }
            return Visibility.Visible;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                return System.Convert.ToInt32(value); 
            }
            catch (Exception)
            {
                return null;
            }
        }
    }
}
