using System;
using System.Windows.Data;
using RxRexrothGui.Services;
using System.Drawing;
using RX_DigiPrint.Services;
using System.Windows;
using RX_DigiPrint.Models;

namespace RX_DigiPrint.Converters
{
    public class LogTypeColor_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                switch ((ELogType)value)
                {
                    case ELogType.eErrLog: return Brushes.Transparent;
                    case ELogType.eErrWarn: return Brushes.Yellow;
                    case ELogType.eErrCont: return Brushes.Red;
                    case ELogType.eErrStop: return Brushes.Red;
                    case ELogType.eErrAbort: return Brushes.Red;
                }
                return null;
            }
            catch
            {
                Console.WriteLine("Exception in LogTypeColor_Converter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public class LogTypeColorButton_Converter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                FrameworkElement targetElement = values[0] as FrameworkElement;

                ELogType logType = ELogType.eErrUndef;

                if (values[1] != DependencyProperty.UnsetValue)
                {      
                        logType = (ELogType)values[1];
                }

                Style newStyle = null;

                switch (logType)
                {
                    case ELogType.eErrUndef: newStyle = (Style)targetElement.TryFindResource("Mouvent.LogEntryColorButtonStyle"); break;
                    case ELogType.eErrLog: newStyle = (Style)targetElement.TryFindResource("Mouvent.LogEntryColorButtonStyle"); break;
                    case ELogType.eErrWarn: newStyle = (Style)targetElement.TryFindResource("Mouvent.LogEntryWarningColorButtonStyle"); break;
                    case ELogType.eErrCont: newStyle = (Style)targetElement.TryFindResource("Mouvent.LogEntryErrorColorButtonStyle"); break;
                    case ELogType.eErrStop: newStyle = (Style)targetElement.TryFindResource("Mouvent.LogEntryErrorColorButtonStyle"); break;
                    case ELogType.eErrAbort: newStyle = (Style)targetElement.TryFindResource("Mouvent.LogEntryErrorColorButtonStyle"); break;
                    default: newStyle = (Style)targetElement.TryFindResource("Mouvent.LogEntryColorButtonStyle"); break;
                }

                if (newStyle == null)
                    newStyle = (Style)targetElement.TryFindResource("Mouvent.LogEntryColorButtonStyle");

                return newStyle;
            }
            catch
            {
                Console.WriteLine("Exception in LogTypeColorButton_Converter");
                return null;
            }
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }


    public class UnboundColumnToLogTypeConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                LogItem item = value as LogItem;
                if (item == null)
                {
                    return null;
                }
                else
                {
                    return item.LogType;
                }
            }
            catch
            {
                Console.WriteLine("Exception in UnboundColumnToLogTypeConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
