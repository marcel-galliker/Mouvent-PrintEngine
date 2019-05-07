using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class UvLampImg_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            Boolean val = (bool) value;

            if(val) return "\\..\\Resources\\Bitmaps\\UV_Lamp.ico";
            else    return "\\..\\Resources\\Bitmaps\\UV_Lamp_OFF.ico";
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
