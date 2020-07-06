using MahApps.Metro.IconPacks;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;

namespace RX_DigiPrint.Converters.Alignment
{
   
    class ColorOffsetUnitConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                if (values == null || values.Length != 2)
                    return null;

                var val = values[0] as DotsCorrectionValue;
                if (val == null) return null;

                bool? unitIsDots = values[1] as bool?;
                if (!unitIsDots.HasValue) return null;

                if (unitIsDots == true) // unit: dots
                {
                    return string.Format("{0:#,0.0}", val.CorrectionInDots);
                }
                else // unit: mm
                {
                    return string.Format("{0:#,0.000}", val.CorrectionInMM);
                }
            }
            catch
            {
                Console.WriteLine("Exception in ColorOffsetUnitConverter");
                return null;
            }
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
        {
            double val = double.Parse(value.ToString(), CultureInfo.InvariantCulture);
            object[] retVal = new object[2];
            retVal[1] = RxGlobals.Alignment.ColorOffsetCorrectionUnitInDots;
            if (RxGlobals.Alignment.ColorOffsetCorrectionUnitInDots == true)
            {
                // value is in dots --> convert to um
                retVal[0] = new DotsCorrectionValue() { Correction = Helpers.UnitConversion.ConvertDotsToUM(1200, val) };
            }
            else // value is in mm
            {
                retVal[0] = new DotsCorrectionValue() { Correction = val * 1000.0 };
            }

            return retVal;
        }
    }


    class RegisterCorrectionsUnitConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                if (values == null || values.Length != 2)
                    return null;

                var val = values[0] as DotsCorrectionValue;
                if (val == null) return null;

                bool? unitIsDots = values[1] as bool?;
                if (!unitIsDots.HasValue) return null;

                if (unitIsDots == true)
                {
                    return string.Format("{0:#,0.0}", val.CorrectionInDots);
                }
                else // unit is mm
                {
                    return string.Format("{0:#,0.000}", val.CorrectionInMM);
                }
            }
            catch
            {
                Console.WriteLine("Exception in RegisterCorrectionsUnitConverter");
                return null;
            }
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                double val = double.Parse(value.ToString(), CultureInfo.InvariantCulture);
                object[] retVal = new object[2];
                retVal[1] = RxGlobals.Alignment.RegisterCorrectionUnitInDots;
                if (RxGlobals.Alignment.RegisterCorrectionUnitInDots == true)
                {
                    // value is in dots --> convert to um
                    retVal[0] = new DotsCorrectionValue() { Correction = Helpers.UnitConversion.ConvertDotsToUM(1200, val) };
                }
                else
                {
                    // value is in mm
                    retVal[0] = new DotsCorrectionValue() { Correction = val * 1000.0 };
                }

                return retVal;
            }
            catch
            {
                Console.WriteLine("Exception in RegisterCorrectionsUnitConverter ConvertBack");
                return null;
            }
        }
    }

    class AngleCorrectionsUnitConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                if (values == null || values.Length != 2)
                    return null;

                var val = values[0] as AngleCorrectionValue;
                if (val == null) return null;

                bool? unitIsRevolutions = values[1] as bool?;
                if (!unitIsRevolutions.HasValue) return null;

                if (unitIsRevolutions == true)
                {
                    
                    return string.Format("{0:#,0.0}", val.CorrectionInRevolutions);
                }
                else
                {
                    return string.Format("{0:#,0.000}", val.CorrectionInMM);
                }
            }
            catch
            {
                Console.WriteLine("Exception in AngleCorrectionsUnitConverter");
                return null;
            }
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                double val = double.Parse(value.ToString(), CultureInfo.InvariantCulture);
                object[] retVal = new object[2];
                retVal[1] = RxGlobals.Alignment.AngleStitchCorrectionsUnitInRevolutions;
                if (RxGlobals.Alignment.AngleStitchCorrectionsUnitInRevolutions == false)
                {
                    // value is in mm --> convert to um
                    retVal[0] = new AngleCorrectionValue() { Correction = val * 1000 };
                }
                else
                {
                    // value is in revolutions --> convert to um
                    retVal[0] = new AngleCorrectionValue()
                    {
                        Correction = Helpers.UnitConversion.ConvertRevolutionsToUm(Helpers.UnitConversion.CorrectionType.AngleCorrection, val)
                    };
                }

                return retVal;
            }
            catch
            {
                Console.WriteLine("Exception in AngleCorrectionsUnitConverter ConvertBack");
                return null;
            }
        }
    }

    
    class StitchCorrectionsUnitConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try

            {
                if (values == null || values.Length != 2)
                    return null;

                var val = values[0] as StitchCorrectionValue;
                if (val == null) return null;

                bool? unitIsRevolutions = values[1] as bool?;
                if (!unitIsRevolutions.HasValue) return null;

                if (unitIsRevolutions == true)
                {
                    return string.Format("{0:#,0.0}", val.CorrectionInRevolutions);
                }
                else
                {
                    return string.Format("{0:#,0.000}", val.CorrectionInMM);
                }
            }
            catch
            {
                Console.WriteLine("Exception in StitchCorrectionsUnitConverter");
                return null;
            }
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                double val = double.Parse(value.ToString(), CultureInfo.InvariantCulture);
                object[] retVal = new object[2];
                retVal[1] = RxGlobals.Alignment.AngleStitchCorrectionsUnitInRevolutions;
                if (RxGlobals.Alignment.AngleStitchCorrectionsUnitInRevolutions == false)
                {
                    // value is in mm --> convert to um
                    retVal[0] = new StitchCorrectionValue() { Correction = val * 1000.0 };
                }
                else
                {
                    // value is in revolutions --> convert to um
                    retVal[0] = new StitchCorrectionValue()
                    {
                        Correction = Helpers.UnitConversion.ConvertRevolutionsToUm(Helpers.UnitConversion.CorrectionType.StitchCorrection, val)
                    };
                }

                return retVal;
            }
            catch
            {
                Console.WriteLine("Exception in StitchCorrectionsUnitConverter ConvertBack");
                return null;
            }
        }
    }
 
    class ScanningMachinesVisibilityConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                if (values == null || values.Length != 2)
                    return null;

                bool? isScanning = values[0] as bool?;
                var collection = values[1] as Helpers.ObservableCollectionEx<BooleanValue>;
                if (collection == null || collection.Count == 0)
                {
                    return Visibility.Hidden;
                }

                int? index = Int32.Parse(parameter.ToString()) as int?;

                if (!isScanning.HasValue || !index.HasValue || collection == null)
                    return null;
                int visibilityIndex = (int)index;
                if (isScanning == true)
                {
                    visibilityIndex = 3 - (int)index;
                }
                if (collection[visibilityIndex].Value == true)
                {
                    return Visibility.Visible;
                }
                else
                {
                    return Visibility.Hidden;
                }
            }
            catch
            {
                Console.WriteLine("Exception in ScanningMachinesVisibilityConverter");
                return null;
            }
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class ScanningMachinesVisibilityConverter2 : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                if (values == null || values.Length != 2)
                    return null;

                bool? isScanning = values[0] as bool?;
                var collection = values[1] as Helpers.ObservableCollectionEx<BooleanValue>;
                if (collection == null || collection.Count == 0)
                {
                    return Visibility.Hidden;
                }

                int? index = Int32.Parse(parameter.ToString()) as int?;

                if (!isScanning.HasValue || !index.HasValue || collection == null)
                    return null;
                int visibilityIndex = (int)index;
                if (isScanning == true)
                {
                    visibilityIndex = 3 - (int)index;
                }
                if (collection[visibilityIndex].Value == true)
                {
                    return Visibility.Visible;
                }
                else
                {
                    return Visibility.Hidden;
                }
            }
            catch
            {
                Console.WriteLine("Exception in ScanningMachinesVisibilityConverter");
                return null;
            }
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class ColorOffsetCorrectionIconConverter : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {
                string result = "ArrowUp";
                if (value is bool)
                {
                    if ((bool)value == true)
                    {
                        result = "ArrowRight";
                    }
                }
                return result;
            }
            catch
            {
                Console.WriteLine("Exception in ColorOffsetCorrectionIconConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }


    class RegisterCorrectionIconConverter : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {
                string result = "ArrowRight";
                if (value is bool)
                {
                    if ((bool)value == true)
                    {
                        result = "ArrowUp";
                    }
                }
                return result;
            }
            catch
            {
                Console.WriteLine("Exception in ColorOffsetCorrectionIconConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class BackwardsRegisterCorrectionIconConverter : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {
                if (value is bool)
                {
                    if ((bool)value == true)
                    {
                        return "ArrowDown";
                    }
                }
                return null;
            }
            catch
            {
                Console.WriteLine("Exception in ColorOffsetCorrectionIconConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class FaultTypeConverter : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {
                RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType faultType =
                    (RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType)value;
                if (faultType == RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue)
                {
                    return "+";
                }
                else if (faultType == RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue)
                {
                    return "-";
                }
                else
                {
                    return "";
                }
            }
            catch
            {
                Console.WriteLine("Exception in FaultTypeConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    

    class BooleanToHeaderColumnWidthConverter : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {


                string result = "auto";
                if (value is bool)
                {
                    if ((bool)value == true)
                    {
                        result = "150";
                    }
                }
                return result;
            }
            catch
            {
                Console.WriteLine("Exception in BooleanToHeaderColumnWidthConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class BooleanAndToVisibilityConverter : IMultiValueConverter
    {
        public object Convert(object[] values,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {
                bool visible = true;
                foreach (object value in values)
                    if (value is bool)
                        visible = visible && (bool)value;

                if (visible)
                    return System.Windows.Visibility.Visible;
                else
                    return System.Windows.Visibility.Collapsed;
            }
            catch
            {
                Console.WriteLine("Exception in BooleanAndToVisibilityConverter");
                return null;
            }
        }

        public object[] ConvertBack(object value,
                                    Type[] targetTypes,
                                    object parameter,
                                    System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    



    class BooleanOrToVisibilityConverter : IMultiValueConverter
    {
        public object Convert(object[] values,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {


                bool visible = false;
                foreach (object value in values)
                {
                    if (value is bool)
                    {
                        visible |= (bool)value;
                    }
                }

                if (visible)
                {
                    return System.Windows.Visibility.Visible;
                }
                else
                {
                    return System.Windows.Visibility.Collapsed;
                }
            }
            catch
            {
                Console.WriteLine("Exception in BooleanOrToVisibilityConverter");
                return null;
            }
        }

        public object[] ConvertBack(object value,
                                    Type[] targetTypes,
                                    object parameter,
                                    System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class BooleanAndNotToVisibilityConverter : IMultiValueConverter
    {
        public object Convert(object[] values,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {
                if (values == null || values.Length != 2)
                    return null;

                bool firstBoolean = (bool)values[0];
                bool secondBoolean = (bool)values[1];

                if (firstBoolean == true && secondBoolean == false)
                {
                    return System.Windows.Visibility.Visible;
                }
                else
                {
                    return System.Windows.Visibility.Collapsed;
                }
            }
            catch
            {
                Console.WriteLine("Exception in BooleanAndNotToVisibilityConverter");
                return null;
            }
        }

        public object[] ConvertBack(object value,
                                    Type[] targetTypes,
                                    object parameter,
                                    System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class BooleanToVisibleOrHiddenConverter : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {
                if (value is bool)
                {
                    if ((bool)value == true)
                    {
                        return System.Windows.Visibility.Visible;
                    }
                    else
                    {
                        return System.Windows.Visibility.Hidden;
                    }
                }
                else
                {
                    throw new Exception("Invalid input");
                }
            }
            catch
            {
                Console.WriteLine("Exception in BooleanToVisibleOrHiddenConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class BooleanNotToVisibilityConverter : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {
                if (value is bool)
                {
                    if ((bool)value == true)
                    {
                        return System.Windows.Visibility.Collapsed;
                    }
                    else
                    {
                        return System.Windows.Visibility.Visible;
                    }
                }
                else
                {
                    throw new Exception("Invalid input");
                }
            }
            catch
            {
                Console.WriteLine("Exception in BooleanNotToVisibilityConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class BooleanNotToVisibilityHideConverter : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            try
            {


                if (value is bool)
                {
                    if ((bool)value == true)
                    {
                        return System.Windows.Visibility.Hidden;
                    }
                    else
                    {
                        return System.Windows.Visibility.Visible;
                    }
                }
                else
                {
                    throw new Exception("Invalid input");
                }
            }
            catch
            {
                Console.WriteLine("Exception in BooleanNotToVisibilityHideConverter");
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class ExpandSectionIconConverter : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            bool? expand = (bool?)value;
            if (expand.HasValue)
            {
                if ((bool)expand == true)
                {
                    return PackIconMaterialKind.ChevronDown;
                }
            }

            return PackIconMaterialKind.ChevronRight;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class ExitDialogIconConverter : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
        {
            bool? changed = (bool?)value;
            if (changed.HasValue)
            {
                if ((bool)changed == false)
                {
                    return PackIconMaterialKind.Check;
                }
            }

            return PackIconMaterialKind.Close;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
