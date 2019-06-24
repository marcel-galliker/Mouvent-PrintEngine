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
                if (val==0)  return "FLUSH";
                else return "INK";
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
