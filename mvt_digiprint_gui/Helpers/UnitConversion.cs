using System;

namespace RX_DigiPrint.Helpers
{
    class UnitConversion
    {
        public enum CorrectionType
        {
            AngleCorrection,
            StitchCorrection,
        }

        public const double AngleUmToRevolutionsFactor = (double)(1.0 / 5.0);
        public const double StitchUmToRevolutionsFactor = (double)(1.0 / 11.0);

        public static double ConvertDotsToUM(int dpi, double valueInDots)
        {
            double result = 0.0;
            try
            {
                result = (double)(valueInDots * (25.4 / dpi) * 1000.0);
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception: " + e.Message);
            }
            return result;
        }

        public static double ConvertUMToDots(int dpi, double valueInUM)
        {
            double result = 0.0;
            try
            {
                result = (double)(valueInUM / 1000.0 * (dpi / 25.4));
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception: " + e.Message);
            }
            return result;
        }

        public static double ConvertUMToMM(double valueInUM)
        {
            return valueInUM / 1000;
        }

        public static double ConvertMMToUM(double valueInMM)
        {
            return valueInMM * 1000;
        }

        public static double ConvertRevolutionsToUm(CorrectionType type, double valueInRevolutions)
        {
            if (type == CorrectionType.AngleCorrection)
            {
                return valueInRevolutions / AngleUmToRevolutionsFactor;
            }
            else if (type == CorrectionType.StitchCorrection)
            {
                return valueInRevolutions / StitchUmToRevolutionsFactor;
            }
            return 0;
        }

        public static double ConvertUmToRevolutions(CorrectionType type, double valueInUm)
        {
            if (type == CorrectionType.AngleCorrection)
            {
                return valueInUm * AngleUmToRevolutionsFactor;
            }
            else if (type == CorrectionType.StitchCorrection)
            {
                return valueInUm * StitchUmToRevolutionsFactor;
            }
            return 0;
        }
    }
}
