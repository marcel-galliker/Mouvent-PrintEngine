using System;
using System.Windows.Data;
using RxRexrothGui.Services;
using System.Drawing;

namespace RX_DigiPrint.Converters
{
    public class BooleanToSideSelectionHeightConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            string result = "auto";
            if (value is bool)
            {
                if ((bool)value == true)
                {
                    result = "30";
                }
            }
            return result;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
