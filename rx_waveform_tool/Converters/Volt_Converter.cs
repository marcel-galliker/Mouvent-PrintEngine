using System;
using System.Windows.Data;
using RxWfTool.Models;

namespace RxWfTool.Converters
{
    public class Volt_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                double val = System.Convert.ToInt32(value);
                return string.Format("{0:F1}", -val);
            }
            catch (Exception)
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
