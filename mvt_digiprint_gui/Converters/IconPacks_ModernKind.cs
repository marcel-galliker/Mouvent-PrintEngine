using MahApps.Metro.IconPacks;
using RX_Common;
using RX_DigiPrint.Models;
using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class IconPacks_ModernKind : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if(value.Equals("P")) return PackIconModernKind.TransformRotateClockwise;
            if(value.Equals("Q")) return PackIconModernKind.TransformRotateCounterclockwise;
            if(value.Equals("_")) return PackIconModernKind.ArrowRight;
            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return value;
        }
    }
 }
