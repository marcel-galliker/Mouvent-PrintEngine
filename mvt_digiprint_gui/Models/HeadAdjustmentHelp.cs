using RX_Common;
using RX_DigiPrint.Helpers;
using System;
using System.Collections.Generic;

namespace RX_DigiPrint.Models
{
    class InstructionImage
    {
        public string ImageSource { get; set; }
        public string ImageHeader { get; set; }
    }

    public class HeadAdjustmentHelp : RxBindable
    {
        public enum HeadAdjustmentType
        {
            TypeUndefined,
            TypeAngle,
            TypeStitch,
        };

        public HeadAdjustmentType AdjustmentType = HeadAdjustmentType.TypeUndefined;

        private RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType _faultType;
        public RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType FaultType
        {
            set { SetProperty(ref _faultType, value); }
            get { return _faultType; }
        }

        private bool _robotConnected;
        public bool RobotIsConnected
        {
            get { return _robotConnected; }
            set { SetProperty(ref _robotConnected, value); }
        }

        private System.Windows.Media.ImageSource _selectionLeftImageSource;
        public System.Windows.Media.ImageSource SelectionLeftImageSource
        {
            get { return _selectionLeftImageSource; }
            set { SetProperty(ref _selectionLeftImageSource, value); }
        }

        private System.Windows.Media.ImageSource _selectionRightImageSource;
        public System.Windows.Media.ImageSource SelectionRightImageSource
        {
            get { return _selectionRightImageSource; }
            set { SetProperty(ref _selectionRightImageSource, value); }
        }

        private System.Windows.Media.ImageSource _robotConnectedImagePath;
        public System.Windows.Media.ImageSource RobotConnectedImagePath
        {
            get { return _robotConnectedImagePath; }
            set { SetProperty(ref _robotConnectedImagePath, value); }
        }

        private double _absolutCorrectionValue;
        public double AbsolutCorrectionValue
        {
            get
            {
                return _absolutCorrectionValue;
            }
            set
            {
                _absolutCorrectionValue = Math.Abs(value); // force non negative values
                OnPropertyChanged("AbsolutCorrectionValue");
                OnPropertyChanged("CorrectionValueAsString");
            }
        }

        private double _correctionValue;
        public double CorrectionValue
        {
            get
            {
                return _correctionValue;
            }
            set
            {
                AbsolutCorrectionValue = Math.Abs(value);
                SetProperty(ref _correctionValue, value);

                if (value == 0)
                {
                    FaultType = RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeUndefined;
                }
                else if (value > 0)
                {
                    FaultType = RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue;
                }
                else
                {
                    FaultType = RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue;
                }


                OnPropertyChanged("CorrectionValueAsString");
            }
        }

        public string CorrectionValueAsString
        {
            get
            {
                string correctionValueAsString = "";
                switch (AdjustmentType)
                {
                    case HeadAdjustmentType.TypeAngle:
                        {
                            if (!RobotIsConnected)
                            {
                                correctionValueAsString +=
                                   UnitConversion.ConvertUmToRevolutions(RX_DigiPrint.Helpers.UnitConversion.CorrectionType.AngleCorrection, CorrectionValue).ToString("N2");
                                correctionValueAsString += " Revolutions (" + CorrectionValue.ToString("N2") + " μm)";
                            }
                            else
                            {
                                if (FaultType == RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue)
                                    correctionValueAsString += "μm (+" + UnitConversion.ConvertUmToRevolutions(RX_DigiPrint.Helpers.UnitConversion.CorrectionType.AngleCorrection, AbsolutCorrectionValue).ToString("N3")
                                        + " Revolutions)";
                                else if (FaultType == RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue)
                                    correctionValueAsString += "μm (-" + UnitConversion.ConvertUmToRevolutions(RX_DigiPrint.Helpers.UnitConversion.CorrectionType.AngleCorrection, AbsolutCorrectionValue).ToString("N3")
                                        + " Revolutions)";
                            }
                            break;
                        }

                    case HeadAdjustmentType.TypeStitch:
                        {
                            if (!RobotIsConnected)
                            {
                                correctionValueAsString +=
                                    UnitConversion.ConvertUmToRevolutions(RX_DigiPrint.Helpers.UnitConversion.CorrectionType.StitchCorrection, CorrectionValue).ToString("N2");
                                correctionValueAsString += " Revolutions (" + CorrectionValue.ToString("N2") + " μm)";
                            }
                            else
                            {
                                if (FaultType == RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue)
                                    correctionValueAsString += "μm (+" + UnitConversion.ConvertUmToRevolutions(RX_DigiPrint.Helpers.UnitConversion.CorrectionType.StitchCorrection, AbsolutCorrectionValue).ToString("N3")
                                        + " Revolutions)";
                                else if (FaultType == RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue)
                                    correctionValueAsString += "μm (-" + UnitConversion.ConvertUmToRevolutions(RX_DigiPrint.Helpers.UnitConversion.CorrectionType.StitchCorrection, AbsolutCorrectionValue).ToString("N3")
                                        + " Revolutions)";
                            }
                            break;

                        }
                    default:
                        break;
                }
                return correctionValueAsString;
            }
        }

        private bool _hasRecommendedValue;
        public bool HasRecommendedValue
        {
            get { return _hasRecommendedValue; }
            set { SetProperty(ref _hasRecommendedValue, value); }
        }

        private bool _showSelectionView;
        public bool ShowSelectionView
        {
            get { return _showSelectionView; }
            set { SetProperty(ref _showSelectionView, value); }
        }

        private string _unit;
        public string Unit
        {
            get { return _unit; }
            set { SetProperty(ref _unit, value); }
        }

        private string _conversionInfoString;
        public string ConversionInfoString
        {
            get { return _conversionInfoString; }
            set { SetProperty(ref _conversionInfoString, value); }
        }
        

        public Dictionary<RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType, List<BitmapInformation>> InstructionsImageSourceList;

        public HeadAdjustmentHelp()
        {
        }
        
        public void Init(HeadAdjustmentType headAdjustmentType,
            double recommendedValue, bool robotConnected)
        {
            ShowSelectionView = (recommendedValue == 0.0) || robotConnected;
            HasRecommendedValue = !ShowSelectionView;
            ConversionInfoString = "";

            AdjustmentType = headAdjustmentType;
            if (AdjustmentType == HeadAdjustmentType.TypeAngle)
            {
                try
                {
                    SelectionLeftImageSource = RxGlobals.AlignmentResources.AngleFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                    SelectionRightImageSource = RxGlobals.AlignmentResources.AngleFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                    InstructionsImageSourceList = RxGlobals.AlignmentResources.AngleCorrectionBitmaps;
                }
                catch
                {
                    // software internal problem!
                    MvtMessageBox.InformationAndExit("Error", "");
                }
                ConversionInfoString = "1 Revolution = 5 μm";
            }
            else if (AdjustmentType == HeadAdjustmentType.TypeStitch)
            {
                try
                {
                    SelectionLeftImageSource = RxGlobals.AlignmentResources.StitchFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                    SelectionRightImageSource = RxGlobals.AlignmentResources.StitchFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                    InstructionsImageSourceList = RxGlobals.AlignmentResources.StitchCorrectionBitmaps;
                }
                catch
                {
                    // software internal problem!
                    MvtMessageBox.InformationAndExit("Error", "");
                }

                ConversionInfoString = "1 Revolution = 11 μm";
            }
            else
            {
                return;
            }

            FaultType = RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeUndefined;

            CorrectionValue = recommendedValue; // um

            if (recommendedValue == 0)
            {
                FaultType = RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeUndefined;
            }
            else if (recommendedValue < 0)
            {
                FaultType = RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue;
            }
            else
            {
                FaultType = RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue;
            }

            RobotIsConnected = robotConnected;
        }

        public void LeftCorrectionImageSelected()
        {
            FaultType = RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue;
            RobotConnectedImagePath = SelectionLeftImageSource;
            ShowSelectionView = false;
            OnPropertyChanged("CorrectionValueAsString");
        }

        public void RightCorrectionImageSelected()
        {
            FaultType = RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue;
            RobotConnectedImagePath = SelectionRightImageSource;
            ShowSelectionView = false;
            OnPropertyChanged("CorrectionValueAsString");
        }


        public void OnOkay()
        {
            if (FaultType == RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue)
            {
                CorrectionValue = AbsolutCorrectionValue;
            }
            else if (FaultType == RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue)
            {
                CorrectionValue = -1 * AbsolutCorrectionValue;
            }
        }

        public void ShowSelection()
        {
            ShowSelectionView = true;
            CorrectionValue = 0.0;
        }

    }
}
