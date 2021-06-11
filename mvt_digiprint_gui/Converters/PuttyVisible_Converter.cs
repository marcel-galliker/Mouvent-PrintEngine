using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Windows;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class PuttyVisible_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            switch((EDeviceType)value)
            {
            case EDeviceType.dev_main:   return Visibility.Visible;
            case EDeviceType.dev_plc:    return Visibility.Hidden;
            case EDeviceType.dev_robot:  return Visibility.Hidden;
            default:                     return Visibility.Visible;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public class PuttyDbgVisible_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            switch((EDeviceType)value)
            {
            case EDeviceType.dev_main:   return Visibility.Hidden;
            case EDeviceType.dev_plc:    return Visibility.Hidden;
            case EDeviceType.dev_robot:  return Visibility.Hidden;
            default:                     return Visibility.Visible;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

}
