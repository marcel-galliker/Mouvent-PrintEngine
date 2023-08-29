using RX_DigiPrint.Services;
using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    //--- HeadValve_Converter ----------------------------------------------------------------------
    public class HeadValve_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                int val =  System.Convert.ToInt32(value);
                switch(val)
                {
                    case 0: return "OFF";
                    case 1: return "INK";
                    case 2: return "FLUSH";
                    case 3: return "BOTH";
                }
                return null;
            }
            catch
            {
                return null;
            }
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
