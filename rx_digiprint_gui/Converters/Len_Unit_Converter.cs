using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    //--- Len_Unit_Converter ----------------------------------------------------------------------
    public class Len_Unit_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if (value==null) return null;
            try
            {
                if (parameter.Equals("m0"))
                {
                    CUnit unit = new CUnit("m");
                    return Math.Round(Rx.StrToDouble(value.ToString())*unit.Factor, 0);
                }
                else
                {
                    CUnit unit = new CUnit(parameter.ToString());
                    return Math.Round(Rx.StrToDouble(value.ToString())*unit.Factor, 3);
                }
            }
            catch
            {
                return value.ToString();
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                CUnit unit = new CUnit(parameter.ToString());
                string str;
                if (unit.Factor!=0) str= (Rx.StrToDouble(value.ToString())/unit.Factor).ToString();
                else str= value.ToString();
                return str;
            }
            catch (Exception)
            {
                return null;
            }
        }
    }
}
