using RX_Common;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class CtrlMode_Str : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {


                foreach (RxEnum<EFluidCtrlMode> item in new EN_FluidCtrlList())
                {
                    if (item.Value.Equals(value)) return item.Display;
                }
                return null;
            }
            catch
            {
                Console.WriteLine("Exception in CtrlMode_Str");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
