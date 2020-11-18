using System;
using System.Drawing;
using System.Windows.Data;

namespace RX_DigiPrint.Converters.CamLib
{
    public class CamCapsResolution_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                Point resolution=(Point)value;
                return string.Format("{0}x{1}", resolution.X, resolution.Y);
            }
            catch
            {
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
