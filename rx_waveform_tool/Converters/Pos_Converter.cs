using System;
using System.Windows.Data;
using RxWfTool.Models;

namespace RxWfTool.Converters
{
    public class Pos_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                Int32 val = System.Convert.ToInt32(value);
                return string.Format("{0:F3}", val/80.0);
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
