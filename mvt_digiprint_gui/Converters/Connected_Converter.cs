using RX_Common;
using System;
using System.Windows.Data;
using System.Windows.Media;

namespace RX_DigiPrint.Converters
{
    public class Connected_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                if (System.Convert.ToInt32(value) == 0) return App.Current.Resources["tcp_offline_ico"];
                else return App.Current.Resources["tcp_online_ico"]; //"../Resources/Bitmaps/tcp_online.ico";
            }
            catch
            {
                Console.WriteLine("Exception in Connected_Converter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public class ConnectedIconColorConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            SolidColorBrush brush = null;
            try
            {
                if (System.Convert.ToInt32(value) == 0)
                {
                    brush = (SolidColorBrush)Rx.BrushError;
                }
                else
                {
                    brush = (SolidColorBrush)App.Current.FindResource("Mouvent.MainColorBrush");
                }
            }
            catch
            {
                Console.WriteLine("Exception in ConnectedIconColorConverter");
                return null;
            }

            return brush;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
    public class ConnectedIconConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                if (System.Convert.ToInt32(value) == 0) return "Close";
                else return "Check";
            }
            catch
            {
                Console.WriteLine("Exception in ConnectedIconConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
    

}
