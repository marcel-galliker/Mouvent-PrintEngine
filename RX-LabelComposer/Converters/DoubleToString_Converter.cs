using System;
using System.Windows.Data;

namespace RX_LabelComposer.Converters
{
   public class DoubleToString_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                int val = System.Convert.ToInt32(value);
                if (val==0) return null;
                return "( "+ val.ToString()+" )";
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
