using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
   public class RotateRect_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                int val =  System.Convert.ToInt32(value)/90;
                if (val%2==0) return 80;    // return w-part of parameter
                else return 50;             // return h-part of parameter
            }
            catch
            {
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
