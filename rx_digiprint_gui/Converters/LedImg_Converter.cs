using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class LedImg_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            Boolean val = (bool) value;

            if(val) return "\\..\\Resources\\Bitmaps\\LED_GREEN.ico";
            else    return "\\..\\Resources\\Bitmaps\\LED_GREY.ico";
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
