using RX_DigiPrint.Services;
using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    //--- HeadVal_Converter ----------------------------------------------------------------------
    public class HeadVal_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                int val =  System.Convert.ToInt32(value);
                if (val==TcpIp.INVALID_VALUE)  return "----";
                else if (val==TcpIp.VAL_UNDERFLOW) return "˅˅˅˅";
                else if (val==TcpIp.VAL_OVERFLOW)  return "˄˄˄˄";
                else return val.ToString();
            }
            catch
            {
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                return System.Convert.ToInt32(value); 
            }
            catch (Exception)
            {
                return null;
            }
        }
    }


    //--- HeadVal_Converter10 --------------------------------------------------------------------------------
    public class HeadVal_Converter10 : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                int val =  System.Convert.ToInt32(value);
                if (val==TcpIp.INVALID_VALUE)  return "----";
                else if (val==TcpIp.VAL_UNDERFLOW) return "˅˅˅˅";
                else if (val==TcpIp.VAL_OVERFLOW)  return "˄˄˄˄";
                else if (val>0) return string.Format("{0}.{1}", val/10, val%10);
                else            return string.Format("{0}.{1}", val/10, (-val)%10);
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

    //--- HeadVal_ConverterTemp -----------------------------------------------------------------------
    public class HeadVal_ConverterTemp : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                int val =  System.Convert.ToInt32(value);
                if (val==TcpIp.INVALID_VALUE) return "----";
                else if (val==TcpIp.VAL_UNDERFLOW) return "˅˅˅˅";
                else if (val==TcpIp.VAL_OVERFLOW)  return "˄˄˄˄";
                else if (val>0) return string.Format("{0}.{1}", val/1000, (val%1000)/100);
                else            return string.Format("{0}.{1}", val/1000, ((-val)%1000)/100);
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
