using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class StepperStateImg_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            UInt32 val = (UInt32)value;
            /*
            switch(val)
            {
            case 1: return "\\..\\Resources\\Bitmaps\\LED_GREEN.ico";
            case 2: return "\\..\\Resources\\Bitmaps\\LED_GREY.ico";
            default: return null;
            }
             * */

            switch(val)
            {
            case 1: return "IDLE";
            case 2: return "MOVE UP";
            case 3: return "MV DOWN";
            case 4: return "BLOCKED";
            default: return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
