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
    class InstructionImage
    {
        public string ImageSource { get; set; }
        public string ImageHeader { get; set; }
    }

    /// <summary>
    /// Interaction logic for HeadAdjustmentView.xaml
    /// </summary>
    public partial class HeadAdjustmentView : Window, INotifyPropertyChanged
    {
        public enum HeadAdjustmentType
        {
            TypeUndefined,
            TypeAngle,
            TypeStitch,
            TypeRegister,
            TypeRegisterBackwards,
            TypeColorOffset
        };

        private HeadAdjustmentType AdjustmentType = HeadAdjustmentType.TypeUndefined;

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

        private bool _robotConnected;
        public bool RobotIsConnected
        {
            get { return _robotConnected; }
            set
            {
                _robotConnected = value;
                OnPropertyChanged("RobotConnected");
            }
        }

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

        private System.Windows.Media.ImageSource _robotConnectedImagePath;
        public System.Windows.Media.ImageSource RobotConnectedImagePath
        {
            get
            {
                return _robotConnectedImagePath;
            }
            set

            {
                _robotConnectedImagePath = value;
                OnPropertyChanged("RobotConnectedImagePath");
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

        Dictionary<RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType, List<BitmapInformation>> InstructionsImageSourceList;

        public HeadAdjustmentView(HeadAdjustmentType headAdjustmentType,
            bool scanCheckAvailable, double recommendedValue, bool robotConnected)
        {
            InitializeComponent();
            DataContext = this;

            ScanCheckValuesAvailable = scanCheckAvailable;
            ShowSelectionView = !scanCheckAvailable;
            
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
            }
            else if (AdjustmentType == HeadAdjustmentType.TypeRegister)
            {
                try
                {
                    SelectionLeftImageSource = RxGlobals.AlignmentResources.RegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                    SelectionRightImageSource = RxGlobals.AlignmentResources.RegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                    InstructionsImageSourceList = new Dictionary<AlignmentResources.AlignmentCorrectionType,List<BitmapInformation>>(); // empty
                }
                catch
                {
                    // software internal problem!
                    MvtMessageBox.InformationAndExit("Error", "");
                }
            }
            else if (AdjustmentType == HeadAdjustmentType.TypeRegisterBackwards)
            {
                try
                {
                    SelectionLeftImageSource = RxGlobals.AlignmentResources.BackwardRegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                    SelectionRightImageSource = RxGlobals.AlignmentResources.BackwardRegisterFaultTypeSelectionBitmap[AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                    InstructionsImageSourceList = new Dictionary<AlignmentResources.AlignmentCorrectionType, List<BitmapInformation>>(); // empty
                }
                catch
                {
                    // software internal problem!
                    MvtMessageBox.InformationAndExit("Error", "");
                }
            }
            else if (AdjustmentType == HeadAdjustmentType.TypeColorOffset)
            {
                try
                {
                    if (!RxGlobals.PrintSystem.IsScanning)
                    {
                        SelectionLeftImageSource =
                            RxGlobals.AlignmentResources.ColorOffsetFaultTypeSelectionBitmap[AlignmentResources.MachineType.MachineTypeNonScanning][AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                        SelectionRightImageSource =
                            RxGlobals.AlignmentResources.ColorOffsetFaultTypeSelectionBitmap[AlignmentResources.MachineType.MachineTypeNonScanning][AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                    }
                    else 
                    {
                        SelectionLeftImageSource =
                            RxGlobals.AlignmentResources.ColorOffsetFaultTypeSelectionBitmap[AlignmentResources.MachineType.MachineTypeScanning][AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                        SelectionRightImageSource =
                            RxGlobals.AlignmentResources.ColorOffsetFaultTypeSelectionBitmap[AlignmentResources.MachineType.MachineTypeScanning][AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                    }
                    
                    InstructionsImageSourceList = new Dictionary<AlignmentResources.AlignmentCorrectionType, List<BitmapInformation>>(); // empty
                }
                catch
                {
                    // software internal problem!
                    MvtMessageBox.InformationAndExit("Error", "");
                }
            }
            else
            {
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

                _CreateInstructionsImagePanel();
            }
            
            RobotIsConnected = robotConnected;
        }

        private void SelectionLeftButton_Click(object sender, RoutedEventArgs e)
        {
            FaultType = RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue;
            RobotConnectedImagePath = SelectionLeftImageSource;
            ShowSelectionView = false;
            OnPropertyChanged("CorrectionValueAsString");
            _CreateInstructionsImagePanel();
        }

        private void SelectionRightButton_Click(object sender, RoutedEventArgs e)
        {
            FaultType = RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue;
            RobotConnectedImagePath = SelectionRightImageSource;
            ShowSelectionView = false;
            OnPropertyChanged("CorrectionValueAsString");
            _CreateInstructionsImagePanel();
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

        private void _CreateInstructionsImagePanel()
        {
            if (FaultType == RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypeUndefined)
            {
                return;
            }

            InstructionsImageStackPanel.Children.Clear();

            try
            {
                if (FaultType == RX_DigiPrint.Models.AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue)
                {
                    var imageSource = InstructionsImageSourceList[AlignmentResources.AlignmentCorrectionType.TypePositiveCorrectionValue];
                    for (int i = 0; i < imageSource.Count; i++)
                    {
                        InstructionsView view = new InstructionsView(imageSource[i].ImageSource, imageSource[i].ImageHeader);
                        InstructionsImageStackPanel.Children.Add(view);
                    }
                }
                else // AlignmentFaultType.TypeRight
                {
                    var imageSource = InstructionsImageSourceList[AlignmentResources.AlignmentCorrectionType.TypeNegativeCorrectionValue];
                    for (int i = 0; i < imageSource.Count; i++)
                    {
                        InstructionsView view = new InstructionsView(imageSource[i].ImageSource, imageSource[i].ImageHeader);
                        InstructionsImageStackPanel.Children.Add(view);
                    }
                }
            }
            catch (Exception)
            {
                // software internal problem!
                MvtMessageBox.InformationAndExit("Error", "");
            }
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

        private void RobotButton_Click(object sender, RoutedEventArgs e)
        {
            // todo: send to robot
        }
    }

}
