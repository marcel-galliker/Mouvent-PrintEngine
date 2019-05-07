using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class Connected_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if (System.Convert.ToInt32(value) == 0) return App.Current.Resources["tcp_offline_ico"];
            else return App.Current.Resources["tcp_online_ico"]; //"../Resources/Bitmaps/tcp_online.ico";
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
