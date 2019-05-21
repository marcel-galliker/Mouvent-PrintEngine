using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class ScanLength_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            PrintQueueItem item = value as PrintQueueItem;
            if (item!=null)
            {
                if (item.SrcPages>1)                       return string.Format("{0} pg", (item.LastPage-item.FirstPage+1)*item.Copies); 
                if (item.LengthUnit==EPQLengthUnit.copies) return string.Format("{0} cp", item.ScanLength);
                if (item.LengthUnit==EPQLengthUnit.mm)
                {
                    if (item.ScanLength<10) return string.Format("{0:n3}m", item.ScanLength);
                    else                    return string.Format("{0:n1}m", item.ScanLength);
                }
            }
            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
