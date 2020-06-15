using RX_DigiPrint.Models;
using System;
using System.Windows.Data;
using System.Windows.Media;
using System.Drawing;

namespace RX_DigiPrint.Converters
{
    public class InkType_ColorBG : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                InkType inkType = value as InkType;
                if (inkType == null) return null;
                return new SolidColorBrush(inkType.Color);
            }
            catch
            {
                Console.WriteLine("Exception in InkType_ColorBG");
                return null;
            }
        }


        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return value;
        }
    }

    public class InkType_ColorBG_Stroke : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                InkType inkType = value as InkType;
                if (inkType == null) return null;
                if (inkType.Color.Equals(System.Windows.Media.Colors.White))
                {
                    return "Gray";
                }
                else
                {
                    return "Transparent";
                }
            }
            catch
            {
                Console.WriteLine("Exception in InkType_ColorBG_Stroke");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return value;
        }
    }
    public class PrintHeadViewStrokeConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                System.Windows.Media.Brush color = value as SolidColorBrush;
                if (color == null) return null;

                if (color.ToString() == "#FFFFFFFF")
                {
                    return "Gray";
                }
                else
                {
                    return "Transparent";
                }
            }
            catch
            {
                Console.WriteLine("Exception in PrintHeadViewStrokeConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return value;
        }
    }
    
    public class InkType_ColorFG : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                InkType inkType = value as InkType;
                if (inkType == null) return null;
                return new SolidColorBrush(inkType.ColorFG);
            }
            catch
            {
                Console.WriteLine("Exception in InkType_ColorFG");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return value;
        }
    }
 }
