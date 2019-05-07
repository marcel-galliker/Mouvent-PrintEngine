using Infragistics.Controls.Grids;
using RX_DigiPrint.Properties;
using RxRexrothGui.Models;
using RxRexrothGui.Services;
using System;
using System.Drawing;
using System.Resources;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace RX_DigiPrint.Converters
{
    public class PlcLogState_Converter : IValueConverter
    {        
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {           
            UnboundColumnDataContext val = value as UnboundColumnDataContext;
            if  (val==null) return null;
            CPlcLogItem item = val.RowData as CPlcLogItem;
            if (item==null) return null;
            switch (item.State)
            {
            case RxRexroth.EnPlcLogState.active:        
                    if ((item.ErrNo&0xf0000000) == 0xf0000000) return (BitmapImage)new FrameworkElement().FindResource("Error_ico");
                    if ((item.ErrNo&0xe0000000) == 0xe0000000) return (BitmapImage)new FrameworkElement().FindResource("Warning_ico");
                    return (BitmapImage)new FrameworkElement().FindResource("Information_ico");

            case RxRexroth.EnPlcLogState.passive:
                    if ((item.ErrNo&0xf0000000) == 0xf0000000) return (BitmapImage)new FrameworkElement().FindResource("ErrorGrey_ico");
                    if ((item.ErrNo&0xe0000000) == 0xe0000000) return (BitmapImage)new FrameworkElement().FindResource("WarningGrey_ico");
                    return (BitmapImage)new FrameworkElement().FindResource("Information_ico");

            case RxRexroth.EnPlcLogState.reset:     return (BitmapImage)new FrameworkElement().FindResource("LogReset_ico");
            case RxRexroth.EnPlcLogState.message:   return (BitmapImage)new FrameworkElement().FindResource("Information_ico");
            default: return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public class PlcLogNo_Converter : IValueConverter
    {        
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {           
            return string.Format("{0:X}", (uint)value);
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
