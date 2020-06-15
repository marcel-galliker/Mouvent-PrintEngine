using RX_Common;

namespace RX_DigiPrint.Helpers
{
    public class CorrectionValue : RxBindable
    {
        protected const double _StrokeDist = 25.4 / 1200;

        protected double _correction;     // in µm (used in comm. with main_ctrl)
        public double Correction        // in µm (used in comm. with main_ctrl)
        {
            get
            {
                return _correction;
            }
            set
            {
                _correction = value;
                OnPropertyChanged("Correction");
            }
        }
    }

    public class DotsCorrectionValue : CorrectionValue
    {
        public double CorrectionInDots
        {
            get
            {
                return (_correction / 1000) / _StrokeDist;
            }
        }
    }

    public class AngleCorrectionValue : CorrectionValue
    {
        public double CorrectionInRevolutions
        {
            get
            {
                // convert um to revolutions
                return Helpers.UnitConversion.ConvertUmToRevolutions(Helpers.UnitConversion.CorrectionType.AngleCorrection, _correction);
            }
        }
    }

    public class StitchCorrectionValue : CorrectionValue
    {
        public double CorrectionInRevolutions
        {
            get
            {
                return Helpers.UnitConversion.ConvertUmToRevolutions(Helpers.UnitConversion.CorrectionType.StitchCorrection, _correction);
            }
        }
    }
}
