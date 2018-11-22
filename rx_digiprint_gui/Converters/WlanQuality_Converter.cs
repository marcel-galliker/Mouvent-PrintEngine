using System;
using System.Windows.Data;
using RxRexrothGui.Services;
using System.Drawing;
using System.Windows.Media.Imaging;

namespace RX_DigiPrint.Converters
{
    public class WlanQuality_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                switch (((UInt32)value+10)/25)
                {
                    case 0:
                    case 1: return (BitmapSource)App.Current.Resources["wlan_1_ico"];
                    case 2: return (BitmapSource)App.Current.Resources["wlan_2_ico"]; 
                    case 3: return (BitmapSource)App.Current.Resources["wlan_3_ico"]; 
                    case 4: return (BitmapSource)App.Current.Resources["wlan_4_ico"]; 
                }
            }
            catch(Exception){};
            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
