using System;
using System.Windows.Data;
using RxRexrothGui.Services;
using System.Drawing;
using System.Globalization;

namespace RX_DigiPrint.Converters
{
    public class Or_Converter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            try
            {
                int length=values.Length;
                int i;
                for (i=0; i<length; i++)
                {
                    if ((bool)values[i]) return true;
                } 
                return false;
            }
            catch
            {
                Console.WriteLine("Exception in Or_Converter");
                return null;
            }
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
