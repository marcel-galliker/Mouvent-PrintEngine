using Org.BouncyCastle.Asn1.Ocsp;
using RX_Common;
using RX_DigiPrint.Helpers;
using System;
using System.Globalization;

namespace RX_DigiPrint.Models
{
    public class SoftwareValueCorrection : RxBindable
    {
        public enum SoftwareValueType
        {
            TypeUndefined,
            TypeRegister,
            TypeRegisterBackwards,
            TypeColorOffset
        };

        private SoftwareValueType ValueType = SoftwareValueType.TypeUndefined;

        private RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType _faultType;
        public RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType FaultType
        {
            set { SetProperty(ref _faultType, value); }
            get { return _faultType; }
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

        private System.Windows.Media.ImageSource _instructionsImagePath;
        public System.Windows.Media.ImageSource InstructionsImagePath
        {
            get { return _instructionsImagePath; }
            set { SetProperty(ref _instructionsImagePath, value); }
        }

        private bool _unitIsDots;
        public bool UnitIsDots
        {
            get { return _unitIsDots; }
            set { SetProperty(ref _unitIsDots, value); }
        }

        private DotsCorrectionValue _correctionValue;
        public DotsCorrectionValue CorrectionValue
        {
            get { return _correctionValue; }
            set
            {
                SetProperty(ref _correctionValue, value);

                AbsolutCorrectionValue = value;

                OnPropertyChanged("CorrectionValueAsString");
            }
        }

        public string CorrectionValueAsString
        {
            get
            {
                string correctionValueAsString = "";
                string sign = "+";
                if (FaultType == RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue)
                {
                    sign = "-";
                }
                
                if (UnitIsDots)
                {
                    correctionValueAsString += " dots (" + sign + string.Format("{0:#,0.000}", AbsolutCorrectionValue.CorrectionInMM) + " mm)";
                }
                else // value in mm
                {
                    correctionValueAsString += " mm (" + sign + string.Format("{0:#,0.0}", AbsolutCorrectionValue.CorrectionInDots) + " dots)";
                }
                return correctionValueAsString;
            }
        }

        private string _absolutCorrectionValueAsString;
        public string AbsolutCorrectionValueAsString
        {
            get
            {
                if (UnitIsDots)
                {
                    return string.Format("{0:#,0.0}", AbsolutCorrectionValue.CorrectionInDots);
                }
                else
                {
                    return string.Format("{0:#,0.000}", AbsolutCorrectionValue.CorrectionInMM);
                }
            }

            set 
            {
                double val = double.Parse(value.ToString(), CultureInfo.InvariantCulture);
                if (val < 0)
                {
                    return;
                }

                if (UnitIsDots)
                {
                    CorrectionValue = new DotsCorrectionValue { Correction = Sign * Helpers.UnitConversion.ConvertDotsToUM(1200, val) };
                }
                else // given in mm
                {
                    CorrectionValue = new DotsCorrectionValue { Correction = Sign * Helpers.UnitConversion.ConvertMMToUM(val) };
                }

                SetProperty(ref _absolutCorrectionValueAsString, value);
            }
        }


        private DotsCorrectionValue _absolutCorrectionValue;
        public DotsCorrectionValue AbsolutCorrectionValue // always in um (only "as string" we adjust units)
        {
            get { return _absolutCorrectionValue; }
            set 
            {
                DotsCorrectionValue absValue = new DotsCorrectionValue { Correction = Math.Abs(value.Correction) }; // force non negative values
                SetProperty(ref _absolutCorrectionValue, absValue); 
            }
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

        public int Sign { get; set; }

        public SoftwareValueCorrection(SoftwareValueCorrection.SoftwareValueType valueType, double recommendedValue)
        {
            ShowSelectionView = (recommendedValue == 0);
            ValueType = valueType;

            if (ValueType == SoftwareValueType.TypeRegister || ValueType == SoftwareValueType.TypeRegisterBackwards)
            {
                UnitIsDots = RxGlobals.Alignment.RegisterCorrectionUnitInDots;
            }
            else
            {
                UnitIsDots = RxGlobals.Alignment.ColorOffsetCorrectionUnitInDots;
            }

            FaultType = RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeUndefined;

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

            Sign = 1;
            if (FaultType == RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue)
            {
                Sign = -1;
            }


            
            if (ValueType == SoftwareValueType.TypeRegister)
            {
                try
                {
                    SelectionLeftImageSource = RxGlobals.AlignmentResources.RegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                    SelectionRightImageSource = RxGlobals.AlignmentResources.RegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                    
                    if (recommendedValue > 0)
                    {
                        InstructionsImagePath = RxGlobals.AlignmentResources.RegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                    }
                    else if (recommendedValue < 0)
                    {
                        InstructionsImagePath = RxGlobals.AlignmentResources.RegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                    }
                }
                catch
                {
                    // software internal problem!
                    MvtMessageBox.InformationAndExit("Error", "");
                }
            }
            else if (ValueType == SoftwareValueType.TypeRegisterBackwards)
            {
                try
                {
                    SelectionLeftImageSource = RxGlobals.AlignmentResources.BackwardRegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                    SelectionRightImageSource = RxGlobals.AlignmentResources.BackwardRegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                    
                    if (recommendedValue > 0)
                    {
                        InstructionsImagePath = RxGlobals.AlignmentResources.BackwardRegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                    }
                    else if (recommendedValue < 0)
                    {
                        InstructionsImagePath = RxGlobals.AlignmentResources.BackwardRegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                    }
                }
                catch
                {
                    // software internal problem!
                    MvtMessageBox.InformationAndExit("Error", "");
                }
            }
            else if (ValueType == SoftwareValueType.TypeColorOffset)
            {
                try
                {
                    if (RxGlobals.PrintSystem.IsScanning == false)
                    {
                        SelectionLeftImageSource =
                            RxGlobals.AlignmentResources.ColorOffsetFaultTypeSelectionBitmap[AlignmentResources.MachineType.MachineTypeNonScanning][AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                        SelectionRightImageSource =
                            RxGlobals.AlignmentResources.ColorOffsetFaultTypeSelectionBitmap[AlignmentResources.MachineType.MachineTypeNonScanning][AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                        
                        if (recommendedValue > 0)
                        {
                            InstructionsImagePath =
                                RxGlobals.AlignmentResources.ColorOffsetFaultTypeSelectionBitmap[AlignmentResources.MachineType.MachineTypeNonScanning][AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                        }
                        else if (recommendedValue < 0)
                        {
                            InstructionsImagePath =
                                RxGlobals.AlignmentResources.ColorOffsetFaultTypeSelectionBitmap[AlignmentResources.MachineType.MachineTypeNonScanning][AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                        }
                    }
                    else
                    {
                        SelectionLeftImageSource =
                            RxGlobals.AlignmentResources.ColorOffsetFaultTypeSelectionBitmap[AlignmentResources.MachineType.MachineTypeScanning][AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                        SelectionRightImageSource =
                            RxGlobals.AlignmentResources.ColorOffsetFaultTypeSelectionBitmap[AlignmentResources.MachineType.MachineTypeScanning][AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                        
                        if (recommendedValue > 0)
                        {
                            InstructionsImagePath =
                                RxGlobals.AlignmentResources.ColorOffsetFaultTypeSelectionBitmap[AlignmentResources.MachineType.MachineTypeScanning][AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                        }
                        else if (recommendedValue < 0)
                        {
                            InstructionsImagePath =
                                    RxGlobals.AlignmentResources.ColorOffsetFaultTypeSelectionBitmap[AlignmentResources.MachineType.MachineTypeScanning][AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                        }
                    }
                }
                catch
                {
                    // software internal problem!
                    MvtMessageBox.InformationAndExit("Error", "");
                }
            }
            else
            {
                // error 
                return;
            }



            CorrectionValue = new DotsCorrectionValue { Correction = recommendedValue };

        }

        public void LeftCorrectionImageSelected()
        {
            FaultType = RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue;
            Sign = 1;
            
            InstructionsImagePath = SelectionLeftImageSource;
            ShowSelectionView = false;
            OnPropertyChanged("CorrectionValueAsString");
        }

        public void RightCorrectionImageSelected()
        {
            FaultType = RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue;
            Sign = -1;
            InstructionsImagePath = SelectionRightImageSource;
            ShowSelectionView = false;
            OnPropertyChanged("CorrectionValueAsString");
        }

        public void OnOkay()
        {
        }

        public void ShowSelection()
        {
            ShowSelectionView = true;
            // CorrectionValue.Correction = 0.0;
            AbsolutCorrectionValueAsString = "0";
        }
   }
}
