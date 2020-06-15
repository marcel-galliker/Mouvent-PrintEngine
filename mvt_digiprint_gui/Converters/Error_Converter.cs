using System;
using System.Windows.Data;
using RxRexrothGui.Services;
using System.Drawing;
using RX_DigiPrint.Services;

namespace RX_DigiPrint.Converters
{
    public class Error_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                uint enumval = (uint)Enum.Parse(typeof(ETestTableErr), parameter.ToString());
                if (((uint)value & enumval) == enumval) return "ERR";
                return "ok";
            }
            catch
            {
                Console.WriteLine("Exception in Error_Converter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

}
