using RX_DigiPrint.Models;
using System;
using System.Windows.Data;
using System.Windows.Media;

namespace RX_DigiPrint.Converters
{
    public class InkType_ColorBG : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            InkType inkType = value as InkType;
            if (inkType==null) return null;
            return new SolidColorBrush(inkType.Color);
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return value;
        }
    }

    public class InkType_ColorFG : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            InkType inkType = value as InkType;
            if (inkType==null) return null;
            return new SolidColorBrush(inkType.ColorFG);
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return value;
        }
    }
 }
