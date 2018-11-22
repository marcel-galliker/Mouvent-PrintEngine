using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class MacAddr_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            int d0, d1, d2, d3, d4, d5;
      //      if (parameter==null) return null;
            try
            {
                Int64 v = System.Convert.ToInt64(value);
                if (v==0) return null;
                d0 = (int)(v>>40) & 0xff;
                d1 = (int)(v>>32) & 0xff;
                d2 = (int)(v>>24) & 0xff;
                d3 = (int)(v>>16) & 0xff;
                d4 = (int)(v>>8) & 0xff;
                d5 = (int)(v) & 0xff;
                return String.Format("{0:X2}-{1:X2}-{2:X2}-{3:X2}-{4:X2}-{5:X2}", d5, d4, d3, d2, d1, d0);
            }
            catch (Exception)
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
