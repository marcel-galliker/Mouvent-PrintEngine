using System;
using System.Windows.Data;
using RxRexrothGui.Services;
using System.Drawing;

namespace RX_DigiPrint.Converters
{
    public class CleaningState_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                int state = System.Convert.ToInt32(value);
                switch (state)
                {
                    case 0: return "ST_INIT";
                    case 1: return "ST_REF_LIFT";
                    case 2: return "ST_REF_CLEAN";
                    case 3: return "ST_IDLE";
                    case 4: return "ST_PRE_MOVE_POS";
                    case 5: return "ST_PRE_MOVE_POS";
                    case 6: return "ST_CLEAN_PRE_POS";
                    case 7: return "ST_CLEAN_MOVE_POS";
                    case 8: return "ST_LIFT_MOVE_POS";
                    case 9: return "ST_LIFT_CAPPING";
                    case 10: return "ST_WIPE";
                    case 11: return "ST_SCREW_REF";
                    case 12: return "ST_SCREW";
                    default: return string.Format("{0:X}", state);
                }
            }
            catch
            {
                Console.WriteLine("Exception in CleaningState_Converter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
