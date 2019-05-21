using System;
using System.Windows.Data;
using RxRexrothGui.Services;
using System.Drawing;
using RX_DigiPrint.Services;

namespace RX_DigiPrint.Converters
{
    public class LogTypeColor_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            switch((ELogType)value)
            {
                case ELogType.eErrLog:   return Brushes.Transparent;
                case ELogType.eErrWarn:  return Brushes.Yellow;
                case ELogType.eErrCont:  return Brushes.Red;
                case ELogType.eErrStop:  return Brushes.Red;
                case ELogType.eErrAbort: return Brushes.Red;
            }
            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
