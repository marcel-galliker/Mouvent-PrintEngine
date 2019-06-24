using RX_Common;
using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class mbar_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                int val = Rx.StrToInt32(value as string);
                if (val > 0) return string.Format("{0}.{1}", val / 10, val % 10);
                else         return string.Format("{0}.{1}", val / 10, (-val) % 10);
            }
            catch (Exception)
            {
                return "---";
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
