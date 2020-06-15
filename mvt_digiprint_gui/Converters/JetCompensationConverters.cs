using RX_DigiPrint.Models;
using System;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;

namespace RX_DigiPrint.Converters
{

    public class JetCompensation_JetsListedConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                ObservableCollection<JetCompensation> list = value as ObservableCollection<JetCompensation>;
                if (list.Count > 0)
                {
                    return Visibility.Collapsed;
                }
                else
                {
                    return Visibility.Visible;
                }
            }
            catch
            {
                Console.WriteLine("Exception in JetCompensation_JetsListedConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public class JetCompensation_ActivatedIconConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                bool val = System.Convert.ToBoolean(value);
                if (val == true)
                {
                    return "Check";
                }
                else
                {
                    return "Close";
                }
            }
            catch
            {
                Console.WriteLine("Exception in JetCompensation_ActivatedIconConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public class JetCompensation_RowColorConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                int val = System.Convert.ToInt32(value);
                if (val % 2 == 0)
                {
                    return (SolidColorBrush)(new BrushConverter().ConvertFrom("#fafafa"));
                }
                else
                {
                    return Brushes.White;
                }
            }
            catch
            {
                Console.WriteLine("Exception in JetCompensation_RowColorConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
