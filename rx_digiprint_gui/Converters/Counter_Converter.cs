using RX_DigiPrint.Models.Enums;
using System;
using System.Windows;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class Counter_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                string str;
                CUnit unit = new CUnit("m");
                double m=System.Convert.ToDouble(value)*unit.Factor;
                if (m<100)      str= String.Format("{0:0.0}", m);           
                else            str = String.Format("{0:0,0}", m);
                return str;
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
