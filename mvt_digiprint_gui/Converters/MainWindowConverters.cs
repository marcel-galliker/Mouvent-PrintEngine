using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Media;

namespace RX_DigiPrint.Converters
{
    public class SelectedTabConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            string selectedTabName = null;
            string currentTabName = null;
            SolidColorBrush brush = null;
            try
            {
                brush = (SolidColorBrush)App.Current.FindResource("Mouvent.TransparentWhiteBrush");
                selectedTabName = value as string;
                currentTabName = parameter as string;
            }
            catch
            {
                return Brushes.Transparent;
            }
            if (selectedTabName.Equals(currentTabName))
            {
                return brush;
            }


            return Brushes.Transparent;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
