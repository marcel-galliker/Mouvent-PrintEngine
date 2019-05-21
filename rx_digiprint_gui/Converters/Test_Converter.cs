using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class Test_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if (value==null) return null;
//            return System.Convert.ToInt32(value);
            return value;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return System.Convert.ToInt32(value);
        }
    }
}
