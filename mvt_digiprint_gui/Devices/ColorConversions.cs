using System;
using System.Collections.Generic;


namespace RX_DigiPrint.Devices
{
    public partial class ColorConversion
    {
        public struct CmykStruct
        {
            public double C;
            public double M;
            public double Y;
            public double K;
        }
        public struct CieLabStruct
        {
            public double L;
            public double a;
            public double b;
        }

		public struct SpectroResultStruct
        {
            public CmykStruct Density;
            public CieLabStruct CieLab;
        }

        public struct RgbStruct
        {
            public byte R;
            public byte G;
            public byte B;
        }

        public struct CieXyzStruct
        {
            public double X;
            public double Y;
            public double Z;
        }

        public enum ReferenceLightningList
        {
            A_Incandescent_Tungsten = 0,
            B_Old_Direct_Sunlight_At_Noon = 1,
            C_Old_Daylight = 2,
            D50_Icc_Profile_PCS = 3,
            D55_Mid_Morning_Daylight = 4,
            D65_Daylight_sRGB = 5,
            D75_North_Sky_Daylight = 6,
            E_Equal_Energy = 7,
            F1_Daylight_Fluorescent = 8,
            F2_Cool_Fluorescent = 9,
            F3_White_Fluorescent = 10,
            F4_Warm_White_Fluorescent = 11,
            F5_Daylight_Fluorescent = 12,
            F6_Lite_White_Fluorescent = 13,
            F7_Daylight_Fluorescent_D65_Simulator = 14,
            F8_Sylvania_F40_D50_Simulator = 15,
            F9_Cool_White_Fluorescent = 16,
            F10_Ultralume_50_Philips_TL85 = 17,
            F11_Ultralume_40_Philips_TL84 = 18,
            F12_Ultralume_30_Philips_TL83 = 19
        }

        public enum ObserverEnum
        {
            Degrees_2 = 0,
            Degrees_10 = 1
        }

        public struct XyzRefStruct
        {
            public double X2;
            public double Y2;
            public double Z2;
            public double X10;
            public double Y10;
            public double Z10;
        }
        private static IReadOnlyList<XyzRefStruct> ReferenceLightningValues = new List<XyzRefStruct>
        {
            //List from http://www.easyrgb.com/en/math.php#text3
            new XyzRefStruct{X2 = 109.850, Y2 = 100.000,Z2 = 035.585, X10 = 111.144, Y10 = 100.000, Z10 = 035.200},   //A_Incandescent_Tungsten
            new XyzRefStruct{X2 = 099.093, Y2 = 100.000,Z2 = 085.313, X10 = 099.178, Y10 = 100.000, Z10 = 084.349},   //B_Old_Direct_Sunlight_At_Noon
            new XyzRefStruct{X2 = 098.074, Y2 = 100.000,Z2 = 118.232, X10 = 097.285, Y10 = 100.000, Z10 = 116.145},   //C_Old_Daylight
            new XyzRefStruct{X2 = 096.422, Y2 = 100.000,Z2 = 082.521, X10 = 096.720, Y10 = 100.000, Z10 = 081.427},   //D50_Icc_Profile_PCS
            new XyzRefStruct{X2 = 095.682, Y2 = 100.000,Z2 = 092.149, X10 = 095.799, Y10 = 100.000, Z10 = 090.926},   //D55_Mid_Morning_Daylight
            new XyzRefStruct{X2 = 095.047, Y2 = 100.000,Z2 = 108.883, X10 = 094.811, Y10 = 100.000, Z10 = 107.304},   //D65_Daylight_sRGB
            new XyzRefStruct{X2 = 094.972, Y2 = 100.000,Z2 = 122.638, X10 = 094.416, Y10 = 100.000, Z10 = 120.641},   //D75_North_Sky_Daylight
            new XyzRefStruct{X2 = 100.000, Y2 = 100.000,Z2 = 100.000, X10 = 100.000, Y10 = 100.000, Z10 = 100.000},   //E_Equal_Energy
            new XyzRefStruct{X2 = 092.834, Y2 = 100.000,Z2 = 103.665, X10 = 094.791, Y10 = 100.000, Z10 = 103.191},   //F1_Daylight_Fluorescent
            new XyzRefStruct{X2 = 099.187, Y2 = 100.000,Z2 = 067.395, X10 = 103.280, Y10 = 100.000, Z10 = 069.026},   //F2_Cool_Fluorescent
            new XyzRefStruct{X2 = 103.754, Y2 = 100.000,Z2 = 049.861, X10 = 108.968, Y10 = 100.000, Z10 = 051.965},   //F3_White_Fluorescent
            new XyzRefStruct{X2 = 109.147, Y2 = 100.000,Z2 = 038.813, X10 = 114.961, Y10 = 100.000, Z10 = 040.963},   //F4_Warm_White_Fluorescent
            new XyzRefStruct{X2 = 090.872, Y2 = 100.000,Z2 = 098.723, X10 = 093.369, Y10 = 100.000, Z10 = 098.636},   //F5_Daylight_Fluorescent
            new XyzRefStruct{X2 = 097.309, Y2 = 100.000,Z2 = 060.191, X10 = 102.148, Y10 = 100.000, Z10 = 062.074},   //F6_Lite_White_Fluorescent
            new XyzRefStruct{X2 = 095.044, Y2 = 100.000,Z2 = 108.755, X10 = 095.792, Y10 = 100.000, Z10 = 107.687},   //F7_Daylight_Fluorescent_D65_Simulator
            new XyzRefStruct{X2 = 096.413, Y2 = 100.000,Z2 = 082.333, X10 = 097.115, Y10 = 100.000, Z10 = 081.135},   //F8_Sylvania_F40_D50_Simulator
            new XyzRefStruct{X2 = 100.365, Y2 = 100.000,Z2 = 067.868, X10 = 102.116, Y10 = 100.000, Z10 = 067.826},   //F9_Cool_White_Fluorescent
            new XyzRefStruct{X2 = 096.174, Y2 = 100.000,Z2 = 081.712, X10 = 099.001, Y10 = 100.000, Z10 = 083.134},   //F10_Ultralume_50_Philips_TL85
            new XyzRefStruct{X2 = 100.966, Y2 = 100.000,Z2 = 064.370, X10 = 103.866, Y10 = 100.000, Z10 = 065.627},   //F11_Ultralume_40_Philips_TL84
            new XyzRefStruct{X2 = 108.046, Y2 = 100.000,Z2 = 039.228, X10 = 111.428, Y10 = 100.000, Z10 = 040.353},   //F12_Ultralume_30_Philips_TL83
        };



        /// <summary>
        /// Calculates DeltaE200 from two Cie L*a*b* values
        /// </summary>
        /// <param name="Lab1">first Cie value</param>
        /// <param name="Lab2">second Cie value</param>
        /// <returns>DeltaE2000</returns>
        public static double DeltaE2000(CieLabStruct Lab1, CieLabStruct Lab2)
        {
            double WHT_L = 1;
            double WHT_C = 1;
            double WHT_H = 1;

            double xC1 = Math.Sqrt(Lab1.a * Lab1.a + Lab1.b * Lab1.b);
            double xC2 = Math.Sqrt(Lab2.a * Lab2.a + Lab2.b * Lab2.b);
            double xCX = (xC1 + xC2) / 2;
            double xGX = 0.5 * (1 - Math.Sqrt(Math.Pow(xCX, 7) / (Math.Pow(xCX, 7) + Math.Pow(25, 7))));
            double xNN = (1 + xGX) * Lab1.a;

            xC1 = Math.Sqrt(xNN * xNN + Lab1.b * Lab1.b);
            double xH1 = CieLabToHue(xNN, Lab1.b);
            xNN = (1 + xGX) * Lab2.a;
            xC2 = Math.Sqrt(xNN * xNN + Lab2.b * Lab2.b);
            double xH2 = CieLabToHue(xNN, Lab2.b);
            double xDL = Lab2.L - Lab1.L;
            double xDC = xC2 - xC1;

            double xDH;
            if ((xC1 * xC2) == 0)
            {
                xDH = 0;
            }
            else
            {
                xNN = Math.Round(xH2 - xH1, 12);
                if (Math.Abs(xNN) <= 180)
                {
                    xDH = xH2 - xH1;
                }
                else
                {
                    if (xNN > 180) xDH = xH2 - xH1 - 360;
                    else xDH = xH2 - xH1 + 360;
                }
            }

            xDH = 2 * Math.Sqrt(xC1 * xC2) * Math.Sin(DegToRad(xDH / 2));
            double xLX = (Lab1.L + Lab2.L) / 2;
            double xCY = (xC1 + xC2) / 2;
            double xHX;
            if ((xC1 * xC2) == 0)
            {
                xHX = xH1 + xH2;
            }
            else
            {
                xNN = Math.Abs(Math.Round(xH1 - xH2, 12));
                if (xNN > 180)
                {
                    if ((xH2 + xH1) < 360) xHX = xH1 + xH2 + 360;
                    else xHX = xH1 + xH2 - 360;
                }
                else
                {
                    xHX = xH1 + xH2;
                }
                xHX /= 2;
            }

            double xTX = 1 - 0.17 * Math.Cos(DegToRad(xHX - 30)) + 0.24
                           * Math.Cos(DegToRad(2 * xHX)) + 0.32
                           * Math.Cos(DegToRad(3 * xHX + 6)) - 0.20
                           * Math.Cos(DegToRad(4 * xHX - 63));
            double xPH = 30 * Math.Exp(-((xHX - 275) / 25) * ((xHX - 275) / 25));
            double xRC = 2 * Math.Sqrt(Math.Pow(xCY, 7) / (Math.Pow(xCY, 7) + Math.Pow(25, 7)));
            double xSL = 1 + ((0.015 * ((xLX - 50) * (xLX - 50)))
                     / Math.Sqrt(20 + ((xLX - 50) * (xLX - 50))));

            double xSC = 1 + 0.045 * xCY;
            double xSH = 1 + 0.015 * xCY * xTX;
            double xRT = -Math.Sin(DegToRad(2 * xPH)) * xRC;

            xDL = xDL / (WHT_L * xSL);
            xDC = xDC / (WHT_C * xSC);
            xDH = xDH / (WHT_H * xSH);

            double DeltaE00 = Math.Sqrt(Math.Pow(xDL, 2) + Math.Pow(xDC, 2) + Math.Pow(xDH, 2) + xRT * xDC * xDH);

            return DeltaE00;
        }

        private static double CieLabToHue(double var_a, double var_b)
        {
            double var_bias = 0;
            if (var_a >= 0 && var_b == 0) return 0;
            if (var_a < 0 && var_b == 0) return 180;
            if (var_a == 0 && var_b > 0) return 90;
            if (var_a == 0 && var_b < 0) return 270;
            if (var_a > 0 && var_b > 0) var_bias = 0;
            if (var_a < 0) var_bias = 180;
            if (var_a > 0 && var_b < 0) var_bias = 360;
            return (RadToDeg(Math.Atan(var_b / var_a)) + var_bias);
        }

        private static double DegToRad(double Degrees)
        {
            //1° × π/180 = 0.01745 rad
            return Degrees * Math.PI / 180;
        }

        private static double RadToDeg(double Radiant)
        {
            //1 rad × 180/π = 57.296°
            return Radiant * 180 / Math.PI;
        }




        /// <summary>
        /// Converts Cie L*a*b* to displayable RGB
        /// </summary>
        /// <param name="Lab">CieLabStruct to be converted</param>
        /// <param name="RefLight">ReferenceLightning, default = D65_Daylight_sRGB</param>
        /// <param name="Observer">Observer, default = 10 degrees</param>
        /// <returns></returns>
        public static System.Windows.Media.Color CieLabToRgb(CieLabStruct Lab,
                                                 ReferenceLightningList RefLight = ReferenceLightningList.D65_Daylight_sRGB, 
                                                 ObserverEnum Observer = ObserverEnum.Degrees_10)
        {
            RgbStruct RGB = XyzToRgb(LabToXyz(Lab, RefLight, Observer));
            return System.Windows.Media.Color.FromArgb(255, RGB.R, RGB.G, RGB.B);
        }

        private static RgbStruct XyzToRgb(CieXyzStruct XYZ)
        {
            //XYZ refer to a D65/2° standard illuminant.
            //RGB output range = 0 ÷ 255

            RgbStruct RGB;

            double var_X = XYZ.X / 100;
            double var_Y = XYZ.Y / 100;
            double var_Z = XYZ.Z / 100;

            double var_R = var_X * 3.2406 + var_Y * -1.5372 + var_Z * -0.4986;
            double var_G = var_X * -0.9689 + var_Y * 1.8758 + var_Z * 0.0415;
            double var_B = var_X * 0.0557 + var_Y * -0.2040 + var_Z * 1.0570;

            if (var_R > 0.0031308) var_R = 1.055 * Math.Pow(var_R, (1 / 2.4)) - 0.055;
            else var_R = 12.92 * var_R;
            if (var_G > 0.0031308) var_G = 1.055 * Math.Pow(var_G, (1 / 2.4)) - 0.055;
            else var_G = 12.92 * var_G;
            if (var_B > 0.0031308) var_B = 1.055 * Math.Pow(var_B, (1 / 2.4)) - 0.055;
            else var_B = 12.92 * var_B;

            var_R *= 255;
            var_G *= 255;
            var_B *= 255;

            RGB.R = (byte)(var_R > 0 ? (var_R > 255 ? 255 : var_R) : 0);
            RGB.G = (byte)(var_G > 0 ? (var_G > 255 ? 255 : var_G) : 0);
            RGB.B = (byte)(var_B > 0 ? (var_B > 255 ? 255 : var_B) : 0);

            return RGB;
        }

        private static CieXyzStruct LabToXyz(CieLabStruct Lab, 
                                      ReferenceLightningList RefLight = ReferenceLightningList.D65_Daylight_sRGB,
                                      ObserverEnum Observer = ObserverEnum.Degrees_10)
        {
            CieXyzStruct XYZ;

            double Epsilon = (double)216 / (double)24389;
            double Kappa = (double)24389 / (double)27;

            double var_Y = (Lab.L + 16) / 116;
            double var_X = (Lab.a / 500) + var_Y;
            double var_Z = var_Y - (Lab.b / 200);

            if (Math.Pow(var_Y, 3) > Epsilon) var_Y = Math.Pow(var_Y, 3);
            else var_Y = (var_Y - 16 / 116) / Kappa;
            if (Math.Pow(var_X, 3) > Epsilon) var_X = Math.Pow(var_X, 3);
            else var_X = (var_X - 16 / 116) / Kappa;
            if (Math.Pow(var_Z, 3) > Epsilon) var_Z = Math.Pow(var_Z, 3);
            else var_Z = (var_Z - 16 / 116) / Kappa;

            XYZ.X = var_X * (Observer == 0 ? ReferenceLightningValues[(int)RefLight].X2 : ReferenceLightningValues[(int)RefLight].X10);
            XYZ.Y = var_Y * (Observer == 0 ? ReferenceLightningValues[(int)RefLight].Y2 : ReferenceLightningValues[(int)RefLight].Y10);
            XYZ.Z = var_Z * (Observer == 0 ? ReferenceLightningValues[(int)RefLight].Z2 : ReferenceLightningValues[(int)RefLight].Z10);

            return XYZ;
        }



    }
}