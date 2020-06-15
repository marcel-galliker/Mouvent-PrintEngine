using RX_DigiPrint.Models;
using System;
using System.Windows.Data;
using System.Windows.Media.Imaging;

namespace RX_DigiPrint.Converters
{
    public class CleafQuality_Converter : IValueConverter
    {
        private static BitmapImage _undef = (BitmapImage)App.Current.FindResource("Question_ico");
        private static BitmapImage _ok    = (BitmapImage)App.Current.FindResource("Yes_ico");
        private static BitmapImage _waste = (BitmapImage)App.Current.FindResource("No_ico");

        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                if (value == null) return _undef;
                if ((bool)value) return _ok;
                return _waste;
            }
            catch
            {
                Console.WriteLine("Exception in NetworkView_IdentifyCheckedConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return value;
        }
    }
 }
