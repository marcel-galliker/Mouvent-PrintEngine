using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class LedImg_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if (value!=null && value.ToString().ToLower().Equals("true")) return "\\..\\Resources\\Bitmaps\\LED_GREEN.ico";
            return "\\..\\Resources\\Bitmaps\\LED_GREY.ico";
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
