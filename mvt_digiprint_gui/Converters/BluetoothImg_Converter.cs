using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class BluetoothImg_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                if ((bool)value) return "\\..\\Resources\\Bitmaps\\Bluetooth.ico";
                else return "\\..\\Resources\\Bitmaps\\Bluetooth_off.ico";
            }
            catch
            {
                Console.WriteLine("Exception in BluetoothImg_Converter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public class BluetoothIcon_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {

                if ((bool)value) return "Bluetooth";
                else return "BluetoothOff";
            }
            catch
            {
                Console.WriteLine("Exception in BluetoothIcon_Converter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
