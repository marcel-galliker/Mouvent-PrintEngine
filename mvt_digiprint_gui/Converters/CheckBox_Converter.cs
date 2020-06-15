using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class CheckBox_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                if (System.Convert.ToInt32(value) == 0) return "";
                else return "ü";
            }
            catch
            {
                Console.WriteLine("Exception in CheckBox_Converter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public class NetworkView_IdentifyCheckedConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                if (System.Convert.ToInt32(value) == 0) return false;
                else return true;
            }
            catch
            {
                Console.WriteLine("Exception in NetworkView_IdentifyCheckedConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    
}
