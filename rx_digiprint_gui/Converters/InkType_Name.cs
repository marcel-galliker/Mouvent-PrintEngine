using RX_DigiPrint.Models;
using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class InkType_Name : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            InkType inkType = value as InkType;
            if (inkType!=null) return inkType.Name;
            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return value;
        }
    }
 }
