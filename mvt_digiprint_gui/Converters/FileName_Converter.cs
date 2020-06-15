using System;
using System.IO;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class FileName_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                if (value == null) return null;
                if (value.Equals("..")) return value;
                return System.IO.Path.GetFileNameWithoutExtension(value as string);
            }
            catch
            {
                Console.WriteLine("Exception in FileName_Converter");
                return null;
            }
        }

            public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
