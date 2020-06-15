using System;
using System.Windows.Data;
using RxRexrothGui.Services;
using System.Drawing;
using System.Globalization;

namespace RX_DigiPrint.Converters
{
    class StartPauseIconConverter : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {
                string result = "Play";
                if (value is bool)
                {
                    if ((bool)value == true)
                    {
                        result = "Pause";
                    }
                }
                return result;
            }
            catch
            {
                Console.WriteLine("Exception in StartPauseIconConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class StartPauseToolTipConverter : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {
                string result = "Start Printing";
                if (value is bool)
                {
                    if ((bool)value == true)
                    {
                        result = "Pause";
                    }
                }
                return result;
            }
            catch
            {
                Console.WriteLine("Exception in StartPauseToolTipConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
    
    class StartPauseMarginConverter : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {
                string result = "6,0,0,0";
                if (value is bool)
                {
                    if ((bool)value == true)
                    {
                        result = "0,0,0,0";
                    }
                }
                return result;
            }
            catch
            {
                Console.WriteLine("Exception in StartPauseMarginConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    
}
