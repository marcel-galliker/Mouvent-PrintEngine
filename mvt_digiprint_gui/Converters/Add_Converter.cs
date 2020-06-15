using System;
using System.Windows.Data;
using RxRexrothGui.Services;
using System.Drawing;

namespace RX_DigiPrint.Converters
{
    public class Add_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                int val =  System.Convert.ToInt32(value);
                int par =  System.Convert.ToInt32(parameter);
                return val+par;
            }
            catch
            {
                Console.WriteLine("Exception in Add_Converter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
