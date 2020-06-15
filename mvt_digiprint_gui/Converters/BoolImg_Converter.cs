using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class BoolImg_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            { Boolean val = (bool)value;

                if (val) return "\\..\\Resources\\Bitmaps\\Confirm.ico";
                else return "";
            }
            catch
            {
                Console.WriteLine("Exception in BoolImg_Converter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
