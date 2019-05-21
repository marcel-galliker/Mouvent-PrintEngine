using RX_LabelComposer.Models;
using System;
using System.Windows.Data;

namespace RX_LabelComposer.Converters
{
   public class LabelPath_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            string path = value as String;
            if (path == null || String.IsNullOrWhiteSpace(path))
            {
                return String.Empty;
            }

            try
            {
                // GetPathRoot returns an empty string if the path specified is a relative
                // path on the current drive or volume.
                bool isRelativeFileNamePath = Equals(System.IO.Path.GetPathRoot(path), String.Empty);
                return isRelativeFileNamePath ? System.IO.Path.Combine(RxGlobals.Dir, path) : path;
            }
            catch
            {
                return String.Empty;
            }
            
            /*// Commented by Samuel
            try
            {
                if (System.IO.Path.GetPathRoot(value as string).Equals("")) return RxGlobals.Dir+(value as string);
                return value as string;
            }
            catch
            {
                 return "";
            }
            */
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
