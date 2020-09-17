using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class ToString_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if (parameter!=null)
            {
                string str=parameter.ToString();
                if (str.Equals("3")) return string.Format("{0:0.000}", value);
                if (str.Equals("X")) return string.Format("0x{0:X}", value);
            }
            return value.ToString();
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
