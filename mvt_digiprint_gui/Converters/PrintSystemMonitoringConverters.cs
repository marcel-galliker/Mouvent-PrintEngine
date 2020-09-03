using RX_Common;
using RX_DigiPrint.Models;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;

namespace RX_DigiPrint.Converters
{
    class InkSupplyStatusColorConverter_Connected : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {
                bool? connected = value as bool?;
                if (connected.HasValue)
                {
                    if ((bool)connected)
                    {
                        return Brushes.SeaGreen;
                    }
                    else
                    {
                        return Rx.BrushError;
                    }
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception: " + e.Message);
            }
            return Brushes.Pink; // for debugging only
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class InkSupplyStatusColorConverter_Int32 : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            int val = -1;
            try
            {
                val =  System.Convert.ToInt32(value);
                if(val == 0)
                {
                    return Brushes.SeaGreen;
                }
                else
                {
                    return Rx.BrushError;
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
                return Brushes.Pink; // for debugging only
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class InkSupplyStatusTextConverter_Connected : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {
                bool? connected = value as bool?;
                if (connected.HasValue)
                {
                    if ((bool)connected)
                    {
                        return "Connected";
                    }
                    else
                    {
                        return "Error";
                    }
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception: " + e.Message);
            }
            return "";
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class InkSupplyStatusColorConverter_UndefinedStatus : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            // this converter is used whenever no status is defined (e.g. no limits are given for a certain value)
            // Might be changed when limits / conditions are given
            return Brushes.SeaGreen;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class InkSupplyStatusTextConverter_Int32 : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            int val = -1;
            try
            {
                val = System.Convert.ToInt32(value);
                if (val == 0)
                {
                    return "Okay";
                }
                else
                {
                    return "Error";
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
                return ""; // for debugging only
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class InkSupplyStatusColorConverter_TempReady : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {
                bool? ready = value as bool?;
                if (ready.HasValue)
                {
                    if ((bool)ready)
                    {
                        return Brushes.SeaGreen;
                    }
                    else
                    {
                        return Rx.BrushWarn;
                    }
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception: " + e.Message);
            }
            return Brushes.Pink; // for debugging only
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class InkCylinderGlobalStatusConverter : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {
                RX_DigiPrint.Views.PrintSystemExtendedView.ETotalStatus inkCylinderStatus =
                    (RX_DigiPrint.Views.PrintSystemExtendedView.ETotalStatus)value;

                if (inkCylinderStatus == Views.PrintSystemExtendedView.ETotalStatus.STATUS_UNDEF)
                {
                    return Brushes.Gray;
                }
                else if (inkCylinderStatus == Views.PrintSystemExtendedView.ETotalStatus.STATUS_OKAY)
                {
                    return Brushes.SeaGreen;
                }
                else if (inkCylinderStatus == Views.PrintSystemExtendedView.ETotalStatus.STATUS_WARNING)
                {
                    return Rx.BrushWarn;
                }
                else if (inkCylinderStatus == Views.PrintSystemExtendedView.ETotalStatus.STATUS_ERROR)
                {
                    return Rx.BrushError;
                }

            }
            catch (Exception e)
            {
                Console.WriteLine("Exception: " + e.Message);
            }
            return Brushes.Gray; // undef
        }

        public object ConvertBack(object value,
                                    Type targetTypes,
                                    object parameter,
                                    System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class FlowResistanceConverter : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {
                double val = (double)value;
                if (val > 1.1)
                {
                    return Rx.BrushError;
                }
                return Brushes.SeaGreen;
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception: " + e.Message);
                return Brushes.Gray; // undef
            }
        }

        public object ConvertBack(object value,
                                    Type targetTypes,
                                    object parameter,
                                    System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
    
}
