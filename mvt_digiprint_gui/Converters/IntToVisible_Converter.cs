using System;
using System.Windows;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class IntToVisible_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if (value == DependencyProperty.UnsetValue)
            {
                Console.WriteLine("IntToVisible_Converter: Value=DependencyProperty.UnsetValue");
                return Visibility.Collapsed;
            }

            try
            {
                if (System.Convert.ToInt32(value)>0) return Visibility.Visible;
                else return Visibility.Collapsed;
            }
            catch
            {
                return Visibility.Collapsed;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
