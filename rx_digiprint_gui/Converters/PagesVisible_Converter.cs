using RX_DigiPrint.Services;
using System;
using System.Windows;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class PagesVisible_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                if (System.Convert.ToInt32(value)>1) return Visibility.Visible;
                /*
                EPrinterType type = (EPrinterType)value;
                if (type==EPrinterType.printer_LB701) 
                if (type==EPrinterType.printer_LB702) return Visibility.Visible;
                else return Visibility.Collapsed;
                 * */
            }
            catch(Exception)
            {
            }
            return Visibility.Collapsed; 
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
