using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class IsBiggerThanConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                var x = System.Convert.ToDouble(parameter);
                var v = System.Convert.ToDouble(value);
                return (v > x);
            }
            catch
            {
                Console.WriteLine("Exception in IsBiggerThanConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
