using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Windows;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    //--- HeadVal_Converter ----------------------------------------------------------------------
    public class HeadVal_Converter : IValueConverter
    {
        public static string _convert(object value)
        {
            if (value == DependencyProperty.UnsetValue)
            {
                return "----";
            }

            try
            {
            	/*
                int val= (int)TcpIp.INVALID_VALUE;
                if (value is int) val = (int)value;
                else if (value is uint) val = (int)value;
                */
                int val = System.Convert.ToInt32(value);
                if (val == TcpIp.INVALID_VALUE) return "----";
                else if (val == TcpIp.VAL_UNDERFLOW) return "˅˅˅˅";
                else if (val == TcpIp.VAL_OVERFLOW) return "˄˄˄˄";
                else return val.ToString();
            }
            catch
            {
                return value.ToString();
            }
        }

        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return _convert(value);
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

    public class HeadVal_SetPoint_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                return " (" + HeadVal_Converter._convert(value) + ")";
            }
            catch
            {
                Console.WriteLine("Exception in HeadVal_SetPoint_Converter");
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
        public static string _convert(object value)
        {
            if (value == DependencyProperty.UnsetValue)
            {
                return "----";
            }

            try
            {
                int val = System.Convert.ToInt32(value);
             //   int val = (int)value;
                if (val==TcpIp.INVALID_VALUE)  return "----";
                else if (val==TcpIp.VAL_UNDERFLOW) return "˅˅˅˅";
                else if (val==TcpIp.VAL_OVERFLOW)  return "˄˄˄˄";
                else if (val>0) return string.Format("{0}.{1}", val/10, val%10);
                else            return string.Format("{0}.{1}", val/10, (-val)%10);
            }
            catch
            {
                return value.ToString();
            }
        }

        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return _convert(value);
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return Rx.StrToInt32(value as string);
        }
    }

    //--- HeadVal_ConverterTemp -----------------------------------------------------------------------
    public class HeadVal_ConverterTemp : IValueConverter
    {
        public static string _convert(object value)
        {
            if (value == DependencyProperty.UnsetValue)
            {
                return "----";
            }

            try
            {
              //  int val = System.Convert.ToInt32((int)value);
                int val = System.Convert.ToInt32(value);
                if (val == TcpIp.INVALID_VALUE) return "----";
                else if (val == TcpIp.VAL_UNDERFLOW) return "˅˅˅˅";
                else if (val == TcpIp.VAL_OVERFLOW) return "˄˄˄˄";
                else if (val > 0) return string.Format("{0}.{1}", val / 1000, (val % 1000) / 100);
                else return string.Format("{0}.{1}", val / 1000, ((-val) % 1000) / 100);
            }
            catch
            {
                return value.ToString();
            }
        }

        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return _convert(value);
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public class HeadVal_SetPoint_ConverterTemp : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                return " (" + HeadVal_ConverterTemp._convert(value) + ")";
            }
            catch
            {
                Console.WriteLine("Exception in HeadVal_SetPoint_ConverterTemp");
                return null;
            }
        }

            public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
    

    class HeadMultiVal_ConverterTemp : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                if (values.Length != 2)
                    return "";

                string val = HeadVal_ConverterTemp._convert(values[0]);
                string setPoint = HeadVal_ConverterTemp._convert(values[1]);
                return val + " (" + setPoint + ")";
            }
            catch
            {
                Console.WriteLine("Exception in HeadMultiVal_ConverterTemp");
                return null;
            }
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class HeadMultiVal_SetPointAsText_Converter10 : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                if (values.Length != 2)
                    return "";

                string val = HeadVal_Converter10._convert(values[0]);
                string setPoint = System.Convert.ToString(values[1]);
                return val + " (" + setPoint + ")";
            }
            catch
            {
                Console.WriteLine("Exception in HeadMultiVal_SetPointAsText_Converter10");
                return null;
            }
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class HeadMultiVal_SetPoint_Converter10 : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                if (values.Length != 2)
                    return "";

                string val = HeadVal_Converter10._convert(values[0]);
                string setPoint = HeadVal_Converter10._convert(values[1]);
                return val + " (" + setPoint + ")";
            }
            catch
            {
                Console.WriteLine("Exception in HeadMultiVal_SetPoint_Converter10");
                return null;
            }
        }

            public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class HeadMultiVal_PumpSpeedConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {

                if (values.Length != 2)
                    return "";

                string val = HeadVal_Converter10._convert(values[0]);
                string setPoint = HeadVal_Converter._convert(values[1]);
                return val + " (" + setPoint + "%)";
            }
            catch
            {
                Console.WriteLine("Exception in HeadMultiVal_PumpSpeedConverter");
                return null;
            }
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class HeadMultiVal_Int_Converter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                if (values.Length != 2)
                    return "";

                string val = System.Convert.ToString(values[0]);
                string setPoint = System.Convert.ToString(values[1]);
                return val + " (" + setPoint + ")";
            }
            catch
            {
                Console.WriteLine("Exception in HeadMultiVal_Int_Converter");
                return null;
            }
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    
}
