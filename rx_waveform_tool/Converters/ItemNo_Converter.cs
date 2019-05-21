using System;
using System.Windows.Data;
using RxWfTool.Models;

namespace RxWfTool.Converters
{
    public class ItemNo_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            WfItem item = value as WfItem;
            if (item==null) return "";
            return item.No;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
