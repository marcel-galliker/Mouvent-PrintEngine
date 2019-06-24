using RX_Common;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class DeviceType_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            EN_DeviceTypeList list = new EN_DeviceTypeList();
            
            foreach (RxEnum<EDeviceType> item in list)
            {
                if (item!=null && item.Value.Equals(value)) return item.Display;
            }
            
            return "not defined";
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
