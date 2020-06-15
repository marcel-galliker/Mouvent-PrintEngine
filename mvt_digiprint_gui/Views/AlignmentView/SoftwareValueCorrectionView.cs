using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows;
using System.Windows.Input;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_Common;

namespace RX_DigiPrint.Views.Alignment
{
    /// <summary>
    /// Interaction logic for HeadAdjustmentView.xaml
    /// </summary>
    public partial class SoftwareValueCorrectionView : Window, INotifyPropertyChanged
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
            set
            {
                _faultType = value;
                OnPropertyChanged("FaultType");
            }
            get
            {
                return _faultType;
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        private System.Windows.Media.ImageSource _selectionLeftImageSource;
        public System.Windows.Media.ImageSource SelectionLeftImageSource
        {
            get
            {
                return _selectionLeftImageSource;
            }
            set            
            {
                _selectionLeftImageSource = value;
                OnPropertyChanged("SelectionLeftImageSource");
            }
        }

        private System.Windows.Media.ImageSource _selectionRightImageSource;
        public System.Windows.Media.ImageSource SelectionRightImageSource
        {
            get
            {
                return _selectionRightImageSource;
            }
            set

            {
                _selectionRightImageSource = value;
                OnPropertyChanged("SelectionLeftImageSource");
            }
        }

        private System.Windows.Media.ImageSource _instructionsImagePath;
        public System.Windows.Media.ImageSource InstructionsImagePath
        {
            get
            {
                return _instructionsImagePath;
            }
            set

            {
                _instructionsImagePath = value;
                OnPropertyChanged("InstructionsImagePath");
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
                _correctionValue = value;
                AbsolutCorrectionValue = Math.Abs(value);

                RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType currentFaultType = FaultType;
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
                if (FaultType != currentFaultType)
                {
                    FaultTypeChanged();
                }

                OnPropertyChanged("CorrectionValue");
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

                switch (ValueType)
                {
                    case SoftwareValueType.TypeRegister:
                    case SoftwareValueType.TypeRegisterBackwards:
                    {
                        if (RxGlobals.Alignment.RegisterCorrectionUnitInDots)
                        {
                            correctionValueAsString += " dots (" + sign + UnitConversion.ConvertDotsToUM(1200, AbsolutCorrectionValue).ToString("N3") + "μm)";
                        }
                        else
                        {
                            correctionValueAsString += " μm (" + sign + UnitConversion.ConvertUMToDots(1200, AbsolutCorrectionValue).ToString("N3") + "dots)";
                        }
                        break;
                    }

                    case SoftwareValueType.TypeColorOffset:
                        if (RxGlobals.Alignment.ColorOffsetCorrectionUnitInDots)
                        {
                            correctionValueAsString += " dots (" + sign + UnitConversion.ConvertDotsToUM(1200, AbsolutCorrectionValue).ToString("N3") + "μm)";
                        }
                        else
                        {
                            correctionValueAsString += " μm (" + sign + UnitConversion.ConvertUMToDots(1200, AbsolutCorrectionValue).ToString("N3") + "dots)";
                        }
                        break;
                    default:
                        break;
                }
                return correctionValueAsString;
            }
        }

        private double _absolutCorrectionValue;
        public double AbsolutCorrectionValue // always in um (only "as string" we adjust units)
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

        private bool _scanCheckValuesAvailable;
        public bool ScanCheckValuesAvailable
        {
            get
            {
                return _scanCheckValuesAvailable;
            }
            set
            {
                _scanCheckValuesAvailable = value;
                OnPropertyChanged("ScanCheckValuesAvailable");
            }
        }

        private bool _showSelectionView;
        public bool ShowSelectionView
        {
            get
            {
                return _showSelectionView;
            }
            set
            {
                _showSelectionView = value;
                OnPropertyChanged("ShowSelectionView");
            }
        }

        private string _unit;
        public string Unit
        {
            get
            {
                return _unit;
            }
            set
            {
                _unit = value;
                OnPropertyChanged("Unit");
            }
        }

        public SoftwareValueCorrectionView(SoftwareValueType valueType,
            bool scanCheckAvailable, double recommendedValue)
        {
            InitializeComponent();
            DataContext = this;

            ScanCheckValuesAvailable = scanCheckAvailable;
            ShowSelectionView = (!scanCheckAvailable) || (recommendedValue == 0);

            ValueType = valueType;
            if (ValueType == SoftwareValueType.TypeRegister)
            {
                try
                {
                    SelectionLeftImageSource = RxGlobals.AlignmentResources.RegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                    SelectionRightImageSource = RxGlobals.AlignmentResources.RegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                    if (ScanCheckValuesAvailable)
                    {
                        if (recommendedValue > 0)
                        {
                            InstructionsImagePath = RxGlobals.AlignmentResources.RegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                        }
                        else if (recommendedValue < 0)
                        {
                            InstructionsImagePath = RxGlobals.AlignmentResources.RegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                        }
                        else
                        {
                            // nothing to do! What image should be shown?
                        }
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
                    if (ScanCheckValuesAvailable)
                    {
                        if (recommendedValue > 0)
                        {
                            InstructionsImagePath = RxGlobals.AlignmentResources.BackwardRegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                        }
                        else if (recommendedValue < 0)
                        {
                            InstructionsImagePath = RxGlobals.AlignmentResources.BackwardRegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                        }
                        else
                        {
                            // nothing to do! What image should be shown?
                        }
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
                        if (ScanCheckValuesAvailable)
                        {
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
                            else
                            {
                                // nothing to do! What image should be shown?
                            }
                        }
                    }
                    else
                    {
                        SelectionLeftImageSource =
                            RxGlobals.AlignmentResources.ColorOffsetFaultTypeSelectionBitmap[AlignmentResources.MachineType.MachineTypeScanning][AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                        SelectionRightImageSource =
                            RxGlobals.AlignmentResources.ColorOffsetFaultTypeSelectionBitmap[AlignmentResources.MachineType.MachineTypeScanning][AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                        if (ScanCheckValuesAvailable)
                        {
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
                            else
                            {
                                // nothing to do! What image should be shown?
                            }
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

            FaultType = RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeUndefined;
            if (scanCheckAvailable)
            {
                CorrectionValue = recommendedValue;

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
            }
        }

        private void SelectionLeftButton_Click(object sender, RoutedEventArgs e)
        {
            FaultType = RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue;
            InstructionsImagePath = SelectionLeftImageSource;
            ShowSelectionView = false;
            OnPropertyChanged("CorrectionValueAsString");
        }

        private void SelectionRightButton_Click(object sender, RoutedEventArgs e)
        {
            FaultType = RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue;
            InstructionsImagePath = SelectionRightImageSource;
            ShowSelectionView = false;
            OnPropertyChanged("CorrectionValueAsString");
        }
        
        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        private void FaultTypeChanged()
        {
            
        }

        private void OkayButton_Click(object sender, RoutedEventArgs e)
        {
            if (FaultType == RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue)
            {
                CorrectionValue = AbsolutCorrectionValue;
            }
            else if (FaultType == RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue)
            {
                CorrectionValue = -1 * AbsolutCorrectionValue;
            }

            DialogResult = true;
        }

        private void GoBackButton_Click(object sender, RoutedEventArgs e)
        {
            ShowSelectionView = true;
            CorrectionValue = 0.0;
        }

        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }
    }

}
