using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Windows;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class PrintQueueCheckVisible_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if(RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_LB702_UV) 
                return Visibility.Visible;
            return Visibility.Collapsed;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public class PrintQueueSaveCancelVisible_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LB702_UV)
            {
                return Visibility.Collapsed;
            }
            else
            {
                if (value == DependencyProperty.UnsetValue)
                {
                    Console.WriteLine("IntToVisible_Converter: Value=DependencyProperty.UnsetValue");
                    return Visibility.Collapsed;
                }

                try
                {
                    if (System.Convert.ToInt32(value) > 0)
                    {
                        return Visibility.Visible;
                    }
                    else return Visibility.Collapsed;
                }
                catch
                {
                    return Visibility.Collapsed;
                }
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
