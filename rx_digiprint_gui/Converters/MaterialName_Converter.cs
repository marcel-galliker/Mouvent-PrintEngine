using RX_DigiPrint.Models;
using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class MaterialName_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            Material material = value as Material;
            if (material==null) return null;
            else                return material.Name;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
