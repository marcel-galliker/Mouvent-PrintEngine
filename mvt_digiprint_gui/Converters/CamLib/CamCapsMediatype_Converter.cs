using System;
using System.Drawing;
using System.Windows.Data;

namespace RX_DigiPrint.Converters.CamLib
{
    public class CamCapsMediatype_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                string mediatype=value.ToString();
                switch (mediatype)
				{
                    case "47504a4d-0000-0010-8000-00aa00389b71": return "MJPG";
                    case "32595559-0000-0010-8000-00aa00389b71": return "YUY2";
                    case "55595659-0000-0010-8000-00AA00389B71": return "YVYU";
                    case "59565955-0000-0010-8000-00AA00389B71": return "UYVY";
                    case "56595559-0000-0010-8000-00AA00389B71": return "YUYV";
                    default: return "???";
				}
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
