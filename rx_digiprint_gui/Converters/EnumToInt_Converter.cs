using RX_DigiPrint.Services;
using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class EnumToInt_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if (value==null) return null;
            return System.Convert.ToInt32(value);
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            int n=System.Convert.ToInt32(value);
            try
            {
                var list=targetType.GetEnumValues();
                var idx = list.GetEnumerator();
                idx.MoveNext();
                while(n>0)
                {
                    idx.MoveNext();
                    n--;
                }
                return idx.Current;
            }
            catch(Exception)
            {
                return n;
            }
        }
    }
}
