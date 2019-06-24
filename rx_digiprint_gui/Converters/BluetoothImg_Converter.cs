using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class BluetoothImg_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if ((bool)value) return "\\..\\Resources\\Bitmaps\\Bluetooth.ico";
            else return "\\..\\Resources\\Bitmaps\\Bluetooth_off.ico";
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
