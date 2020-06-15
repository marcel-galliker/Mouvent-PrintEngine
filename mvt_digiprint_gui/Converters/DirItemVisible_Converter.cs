using RX_DigiPrint.Models;
using System;
using System.Windows;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class DirItemVisible_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                DirItem item = value as DirItem;
                if (item != null)
                {
                    if (item.Time.Equals("")) return Visibility.Visible;
                }
                return Visibility.Collapsed;
            }
            catch
            {
                Console.WriteLine("Exception in DirItemVisible_Converter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
