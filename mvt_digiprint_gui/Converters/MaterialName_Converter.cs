using RX_DigiPrint.Models;
using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class MaterialName_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                Material material = value as Material;
                if (material == null) return null;
                else return material.Name;
            }
            catch
            {
                Console.WriteLine("Exception in MaterialName_Converter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
