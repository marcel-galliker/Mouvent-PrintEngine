using RX_Common;
using RX_DigiPrint.Helpers;
using System;

namespace RX_DigiPrint.Models
{
    public class ClusterAlignment : RxBindable
    {

        public int ClusterNumber { get; set; } // 0, 1, ...
        public int ClusterDisplyNumber { get; set; } // 1, 2, ...
        private string _ClusterName;
        public string ClusterName
        {
            get { return _ClusterName; }
            set { SetProperty(ref _ClusterName, value); }
        }

        private bool _isScanning;
        public bool IsScanning
        {
            get { return _isScanning; }
            set
            {
                _isScanning = value;
                OnPropertyChanged("IsScanning");
                //SetProperty(ref _isScanning, value);
            }
        }

        private bool _robotIsConnected;
        public bool RobotIsConnected
        {
            get { return _robotIsConnected; }
            set
            {
                SetProperty(ref _robotIsConnected, value);
            }
        }

        private bool _isFirstInView;
        public bool IsFirstInView
        {
            get { return _isFirstInView; }
            set
            {
                SetProperty(ref _isFirstInView, value);
            }
        }


        private bool _angleEditMode;
        public bool AngleEditMode
        {
            get { return _angleEditMode; }
            set
            {
                SetProperty(ref _angleEditMode, value);
            }
        }

        private bool _hasStitchEditMode;
        public bool HasStitchEditMode
        {
            get { return _hasStitchEditMode; }
            set
            {
                SetProperty(ref _hasStitchEditMode, value);
            }
        }


        private bool _stitchEditMode;
        public bool StitchEditMode
        {
            get { return _stitchEditMode; }
            set
            {
                SetProperty(ref _stitchEditMode, value);
            }
        }

        private bool _registerEditMode;
        public bool RegisterEditMode
        {
            get { return _registerEditMode; }
            set
            {
                SetProperty(ref _registerEditMode, value);
            }
        }

        private bool _backwardsRegisterEditMode;
        public bool BackwardsRegisterEditMode
        {
            get { return _backwardsRegisterEditMode; }
            set
            {
                SetProperty(ref _backwardsRegisterEditMode, value);
            }
        }

        private bool _jetCompensationEditMode;
        public bool JetCompensationEditMode
        {
            get { return _jetCompensationEditMode; }
            set
            {
                SetProperty(ref _jetCompensationEditMode, value);
            }
        }

        private bool _densityEditMode;
        public bool DensityEditMode
        {
            get { return _densityEditMode; }
            set
            {
                SetProperty(ref _densityEditMode, value);
            }
        }


        private bool _colorOffsetEditMode;
        public bool ColorOffsetEditMode
        {
            get { return _colorOffsetEditMode; }
            set
            {
                SetProperty(ref _colorOffsetEditMode, value);
            }
        }

        private bool _colorOffsetCorrectionUnitInDots;
        public bool ColorOffsetCorrectionUnitInDots
        {
            get { return _colorOffsetCorrectionUnitInDots; }
            set
            {
                SetProperty(ref _colorOffsetCorrectionUnitInDots, value);
                if (ColorOffsetCorrectionUnitInDots == true) ColorOffsetHeader = "Color Offset [dots]";
                else ColorOffsetHeader = "Color Offset [mm]";
            }
        }

        private bool _angleCorrectionUnitInRevolutions;
        public bool AngleStitchCorrectionsUnitInRevolutions
        {
            get { return _angleCorrectionUnitInRevolutions; }
            set
            {
                SetProperty(ref _angleCorrectionUnitInRevolutions, value);
                if (AngleStitchCorrectionsUnitInRevolutions == true)
                {
                    AngleHeader = "Angle [Rev.]";
                    StitchHeader = "Stitch [Rev.]";
                }
                else
                {
                    AngleHeader = "Angle [mm]";
                    StitchHeader = "Stitch [mm]";
                }
               
            }
        }

        private bool _registerCorrectionUnit;
        public bool RegisterCorrectionUnitInDots
        {
            get { return _registerCorrectionUnit; }
            set
            {
                SetProperty(ref _registerCorrectionUnit, value);
                if (RegisterCorrectionUnitInDots == true)
                {
                    RegisterHeader = "Head Distance [dots]";
                    BackwardsRegisterHeader = "Head Distance Backwards [dots]";
                }
                else
                {
                    RegisterHeader = "Head Distance [mm]";
                    BackwardsRegisterHeader = "Head Distance Backwards [mm]";
                }
            }
        }

        private bool _hasVisibleColorOffsetCorrectionValue;
        public bool HasVisibleColorOffsetCorrectionValue
        {
            get { return _hasVisibleColorOffsetCorrectionValue; }
            set
            {
                SetProperty(ref _hasVisibleColorOffsetCorrectionValue, value);
            }
        }

        private Helpers.ObservableCollectionEx<AngleCorrectionValue> _angleCorrectionValues;
        public Helpers.ObservableCollectionEx<AngleCorrectionValue> AngleCorrectionValues
        {
            get { return _angleCorrectionValues; }
            set
            {
                SetProperty(ref _angleCorrectionValues, value);
            }
        }

        private Helpers.ObservableCollectionEx<DotsCorrectionValue> _registerCorrectionValues;
        public Helpers.ObservableCollectionEx<DotsCorrectionValue> RegisterCorrectionValues
        {
            get { return _registerCorrectionValues; }
            set { SetProperty(ref _registerCorrectionValues, value); }
        }
        private Helpers.ObservableCollectionEx<DotsCorrectionValue> _backwardRegisterCorrectionValues;
        public Helpers.ObservableCollectionEx<DotsCorrectionValue> BackwardRegisterCorrectionValues
        {
            get { return _backwardRegisterCorrectionValues; }
            set { SetProperty(ref _backwardRegisterCorrectionValues, value); }
        }

        private Helpers.ObservableCollectionEx<StitchCorrectionValue> _stitchCorrectionValues;
        public Helpers.ObservableCollectionEx<StitchCorrectionValue> StitchCorrectionValues
        {
            get { return _stitchCorrectionValues; }
            set { SetProperty(ref _stitchCorrectionValues, value); }
        }

        private Helpers.ObservableCollectionEx<DotsCorrectionValue> _colorOffsetCorrectionValues;
        public Helpers.ObservableCollectionEx<DotsCorrectionValue> ColorOffsetCorrectionValues
        {
            get { return _colorOffsetCorrectionValues; }
            set { SetProperty(ref _colorOffsetCorrectionValues, value); }
        }

        private Helpers.ObservableCollectionEx<DotsCorrectionValue> _colorOffsetSetValues;
        public Helpers.ObservableCollectionEx<DotsCorrectionValue> ColorOffsetSetValues
        {
            get { return _colorOffsetSetValues; }
            set { SetProperty(ref _colorOffsetSetValues, value); }
        }

        private Helpers.ObservableCollectionEx<DotsCorrectionValue> _registerSetValues;
        public Helpers.ObservableCollectionEx<DotsCorrectionValue> RegisterSetValues
        {
            get { return _registerSetValues; }
            set { SetProperty(ref _registerSetValues, value); }
        }

        private Helpers.ObservableCollectionEx<DotsCorrectionValue> _backwardRegisterSetValues;
        public Helpers.ObservableCollectionEx<DotsCorrectionValue> BackwardRegisterSetValues
        {
            get { return _backwardRegisterSetValues; }
            set { SetProperty(ref _backwardRegisterSetValues, value); }
        }

        private Helpers.ObservableCollectionEx<BooleanValue> _showColorOffset;
        public Helpers.ObservableCollectionEx<BooleanValue> ShowColorOffset
        {
            get { return _showColorOffset; }
            set
            {
                SetProperty(ref _showColorOffset, value);
            }
        }

        private string _colorOffsetHeader = "Color Offset";
        public string ColorOffsetHeader
        {
            get { return _colorOffsetHeader; }
            set
            {
                SetProperty(ref _colorOffsetHeader, value);
            }
        }

        private string _angleHeader = "Angle";
        public string AngleHeader
        {
            get { return _angleHeader; }
            set
            {
                SetProperty(ref _angleHeader, value);
            }
        }

        private string _sticthHeader = "Stitch";
        public string StitchHeader
        {
            get { return _sticthHeader; }
            set
            {
                SetProperty(ref _sticthHeader, value);
            }
        }

        private string _registerHeader = "Head Distance";
        public string RegisterHeader
        {
            get { return _registerHeader; }
            set
            {
                SetProperty(ref _registerHeader, value);
            }
        }


        private string _backwardsRegisterHeader = "Head Distance";
        public string BackwardsRegisterHeader
        {
            get { return _backwardsRegisterHeader; }
            set
            {
                SetProperty(ref _backwardsRegisterHeader, value);
            }
        }

        public ClusterAlignment(int clusterNumber)
        {
            ClusterNumber = clusterNumber;
            ClusterDisplyNumber = clusterNumber + 1;
            ClusterName = "Cluster " + ClusterDisplyNumber;
            AngleCorrectionValues = new Helpers.ObservableCollectionEx<AngleCorrectionValue>();
            StitchCorrectionValues = new Helpers.ObservableCollectionEx<StitchCorrectionValue>();
            RegisterCorrectionValues = new Helpers.ObservableCollectionEx<DotsCorrectionValue>();
            BackwardRegisterCorrectionValues = new Helpers.ObservableCollectionEx<DotsCorrectionValue>();
            ColorOffsetCorrectionValues = new Helpers.ObservableCollectionEx<DotsCorrectionValue>();
            ColorOffsetSetValues = new Helpers.ObservableCollectionEx<DotsCorrectionValue>();
            RegisterSetValues = new Helpers.ObservableCollectionEx<DotsCorrectionValue>();
            BackwardRegisterSetValues = new Helpers.ObservableCollectionEx<DotsCorrectionValue>();
            ShowColorOffset = new Helpers.ObservableCollectionEx<BooleanValue>();
            HasVisibleColorOffsetCorrectionValue = false;
            IsScanning = RxGlobals.PrintSystem.IsTx;
            ColorOffsetEditMode = RxGlobals.Alignment.ColorOffsetEditMode;
            AngleEditMode = RxGlobals.Alignment.AngleEditMode;
            StitchEditMode = RxGlobals.Alignment.StitchEditMode;
            HasStitchEditMode = RxGlobals.Alignment.HasStitchEditMode;
            RegisterEditMode = RxGlobals.Alignment.RegisterEditMode;

        }

        public void Init()
        {
            ColorOffsetCorrectionUnitInDots = RxGlobals.Alignment.ColorOffsetCorrectionUnitInDots;
            AngleStitchCorrectionsUnitInRevolutions = RxGlobals.Alignment.AngleStitchCorrectionsUnitInRevolutions;
            RegisterCorrectionUnitInDots = RxGlobals.Alignment.RegisterCorrectionUnitInDots;

            InitCorrectionValues();
            RobotIsConnected = RxGlobals.Alignment.RobotIsConnected;
        }

        public void SetRobotIsConnected(bool connected)
        {
            RobotIsConnected = connected;
        }

        public void SetIsFirstInView(bool isFirst)
        {
            IsFirstInView = isFirst;
        }
        
        public void SetIsScanning(bool isScanning)
        {
            IsScanning = isScanning;
        }
        
        public void InitCorrectionValues()
        {
            if (RxGlobals.PrintSystem.ColorCnt == 0 || RxGlobals.PrintSystem.InkCylindersPerColor == 0
                   || RxGlobals.PrintSystem.HeadsPerColor == 0)
            {
                // no colors or heads defined
                return;
            }
            if (RxGlobals.PrintSystem.HeadDist == null || RxGlobals.PrintSystem.HeadDistBack == null
                || RxGlobals.PrintSystem.ColorOffset == null)
            {
                // not initialized yet
                return;
            }

            AngleCorrectionValues.Clear();
            StitchCorrectionValues.Clear();
            RegisterCorrectionValues.Clear();
            BackwardRegisterCorrectionValues.Clear();
            ColorOffsetCorrectionValues.Clear();
            ColorOffsetSetValues.Clear();
            RegisterSetValues.Clear();
            BackwardRegisterSetValues.Clear();
            ShowColorOffset.Clear();

            for (int h = 0; h < Alignment.kHeadsPerCluster; h++)
            {
                int globalHeadNumber = 0;
                if (IsScanning)
                {
                    globalHeadNumber = ClusterNumber * Alignment.kHeadsPerCluster + h;
                }
                else
                {
                    globalHeadNumber = ClusterNumber * Alignment.kHeadsPerCluster + h;
                }

                if (globalHeadNumber < 0)
                {
                    RegisterCorrectionValues.Add(new DotsCorrectionValue() { Correction = 0.0 });
                    BackwardRegisterCorrectionValues.Add(new DotsCorrectionValue() { Correction = 0.0 });
                    ShowColorOffset.Add(new BooleanValue() { Value = false });
                    ColorOffsetCorrectionValues.Add(new DotsCorrectionValue() { Correction = 0 });
                    ColorOffsetSetValues.Add(new DotsCorrectionValue() { Correction = 0.0 });
                    RegisterSetValues.Add(new DotsCorrectionValue() { Correction = 0.0 });
                    BackwardRegisterSetValues.Add(new DotsCorrectionValue() { Correction = 0.0 });
                    AngleCorrectionValues.Add(new AngleCorrectionValue() { Correction = 0.0 });
                    StitchCorrectionValues.Add(new StitchCorrectionValue() { Correction = 0.0 });
                }
                else
                {
                    double angleCorrection = 0.0;
                    double stitchCorrection = 0.0;

                    DateTime startTime = DateTime.Now;
                    if (globalHeadNumber < RxGlobals.Alignment.AngleCorrectionValues.Count)
                    {
                        angleCorrection = RxGlobals.Alignment.AngleCorrectionValues[globalHeadNumber].Correction;
                    }
                    if (globalHeadNumber < RxGlobals.Alignment.StitchCorrectionValues.Count)
                    {
                        stitchCorrection = RxGlobals.Alignment.StitchCorrectionValues[globalHeadNumber].Correction;
                    }

                    AngleCorrectionValues.Add(new AngleCorrectionValue() { Correction = angleCorrection });
                    StitchCorrectionValues.Add(new StitchCorrectionValue() { Correction = stitchCorrection });

                    double registerSetValue = 0.0; // in um
                    double backwardRegisterSetValue = 0.0; // in um
                    if (globalHeadNumber < RxGlobals.PrintSystem.HeadDist.Length)
                    {
                        registerSetValue = Helpers.UnitConversion.ConvertMMToUM(RxGlobals.PrintSystem.HeadDist[globalHeadNumber]);
                    }
                    if (globalHeadNumber < RxGlobals.PrintSystem.HeadDistBack.Length)
                    {
                        backwardRegisterSetValue = Helpers.UnitConversion.ConvertMMToUM(RxGlobals.PrintSystem.HeadDistBack[globalHeadNumber]); // RxGlobals.PrintSystem.HeadDistBack is in mm (?)
                    }

                    RegisterSetValues.Add(new DotsCorrectionValue() { Correction = registerSetValue });
                    RegisterCorrectionValues.Add(new DotsCorrectionValue() { Correction = 0.0 });
                    BackwardRegisterSetValues.Add(new DotsCorrectionValue() { Correction = backwardRegisterSetValue });
                    BackwardRegisterCorrectionValues.Add(new DotsCorrectionValue() { Correction = 0.0 });

                    bool showColorOffset = false;
                    
                    showColorOffset = _IsLastPrintHeadOfInkSupply(globalHeadNumber);
                   
                    int inkSupplyNumber = globalHeadNumber / RxGlobals.PrintSystem.HeadsPerInkCylinder;
                    double colorOffsetCorrection = 0;
                    if (inkSupplyNumber < RxGlobals.PrintSystem.ColorOffset.Length)
                    {
                        colorOffsetCorrection = Helpers.UnitConversion.ConvertMMToUM(RxGlobals.PrintSystem.ColorOffset[inkSupplyNumber]); // RxGlobals.PrintSystem.ColorOffset is in mm (?)
                    }

                    if (showColorOffset)
                    {
                        ColorOffsetSetValues.Add(new DotsCorrectionValue() { Correction = colorOffsetCorrection });
                        ColorOffsetCorrectionValues.Add(new DotsCorrectionValue() { Correction = 0.0 });
                    }
                    else
                    {
                        ColorOffsetSetValues.Add(new DotsCorrectionValue() { Correction = 0.0 });
                        ColorOffsetCorrectionValues.Add(new DotsCorrectionValue() { Correction = 0.0 });
                    }
                    ShowColorOffset.Add(new BooleanValue() { Value = showColorOffset });
                }
            }

            if (IsScanning)
            {

                AngleCorrectionValues.ReverseOrder();
                OnPropertyChanged("AngleCorrectionValues");
                StitchCorrectionValues.ReverseOrder();
                OnPropertyChanged("StitchCorrectionValues");
                RegisterCorrectionValues.ReverseOrder();
                OnPropertyChanged("RegisterCorrectionValues");
                BackwardRegisterCorrectionValues.ReverseOrder();
                OnPropertyChanged("BackwardRegisterCorrectionValues");
                ColorOffsetCorrectionValues.ReverseOrder();
                OnPropertyChanged("ColorOffsetCorrectionValues");
                ColorOffsetSetValues.ReverseOrder();
                OnPropertyChanged("ColorOffsetSetValues");
                RegisterSetValues.ReverseOrder();
                OnPropertyChanged("RegisterSetValues");
                BackwardRegisterSetValues.ReverseOrder();
                OnPropertyChanged("BackwardRegisterSetValues");
            }

            HasVisibleColorOffsetCorrectionValue = false;
            for (int i = 0; i < Alignment.kHeadsPerCluster; i++)
            {
                if (ShowColorOffset[i].Value == true)
                {
                    HasVisibleColorOffsetCorrectionValue = true;
                    break;
                }
            }
        }

        public void SaveCorrectionValues()
        {
            for (int h = 0; h < Alignment.kHeadsPerCluster; h++)
            {
                int globalHeadNumber = 0;
                if (IsScanning)
                {
                    globalHeadNumber = ClusterNumber * Alignment.kHeadsPerCluster + h;
                }
                else
                {
                    globalHeadNumber = ClusterNumber * Alignment.kHeadsPerCluster + h;
                }
                if (globalHeadNumber >= 0)
                {
                    int inkSupplyNumber = globalHeadNumber / RxGlobals.PrintSystem.HeadsPerInkCylinder;
                    int index = h;
                    if (IsScanning)
                    {
                        index = 3 - index;
                    }


                    if (globalHeadNumber < RxGlobals.Alignment.AngleCorrectionValues.Count && h < AngleCorrectionValues.Count)
                    {
                        RxGlobals.Alignment.AngleCorrectionValues[globalHeadNumber].Correction = AngleCorrectionValues[h].Correction;
                    }
                    else
                    {
                        Console.WriteLine("Failed to set AngleCorrectionValue for head #" + globalHeadNumber.ToString());
                    }
                    if (globalHeadNumber < RxGlobals.Alignment.StitchCorrectionValues.Count && h < StitchCorrectionValues.Count)
                    {
                        RxGlobals.Alignment.StitchCorrectionValues[globalHeadNumber].Correction = StitchCorrectionValues[h].Correction;
                    }
                    else
                    {
                        Console.WriteLine("Failed to set StitchCorrectionValue for head #" + globalHeadNumber.ToString());
                    }

                    // compute all values in um:
                    double newSetValue = 0.0;
                    if (index < RegisterCorrectionValues.Count && index < RegisterSetValues.Count
                        && globalHeadNumber < RxGlobals.PrintSystem.HeadDist.Length)
                    {
                        newSetValue = RegisterSetValues[index].Correction + RegisterCorrectionValues[index].Correction;
                        RegisterSetValues[index] = new DotsCorrectionValue { Correction = newSetValue };
                        RegisterCorrectionValues[index] = new DotsCorrectionValue { Correction = 0.0 };
                        RxGlobals.PrintSystem.HeadDist[globalHeadNumber] = Helpers.UnitConversion.ConvertUMToMM(RegisterSetValues[index].Correction); // RxGlobals.PrintSystem.HeadDist is in mm (???)
                    }
                    else
                    {
                        Console.WriteLine("Failed to set RegisterCorrectionValue for head #" + globalHeadNumber.ToString());
                    }
                    if (index < BackwardRegisterSetValues.Count && index < BackwardRegisterCorrectionValues.Count
                        && globalHeadNumber < RxGlobals.PrintSystem.HeadDistBack.Length)
                    {
                        newSetValue = BackwardRegisterSetValues[index].Correction + BackwardRegisterCorrectionValues[index].Correction;
                        BackwardRegisterSetValues[index] = new DotsCorrectionValue { Correction = newSetValue };
                        BackwardRegisterCorrectionValues[index] = new DotsCorrectionValue { Correction = 0.0 };
                        RxGlobals.PrintSystem.HeadDistBack[globalHeadNumber] = Helpers.UnitConversion.ConvertUMToMM(BackwardRegisterSetValues[index].Correction); // RxGlobals.PrintSystem.HeadDistBack is in mm (???)
                    }
                    else
                    {
                        Console.WriteLine("Failed to set BackwardRegisterCorrectionValue for head #" + globalHeadNumber.ToString());
                    }

                    bool showColorOffset = false;
                    showColorOffset = _IsLastPrintHeadOfInkSupply(globalHeadNumber);
                   
                    if (showColorOffset)
                    {
                        // compute all values in um:
                        if (index < ColorOffsetSetValues.Count && index < ColorOffsetCorrectionValues.Count &&
                           inkSupplyNumber < RxGlobals.PrintSystem.ColorOffset.Length)
                        {
                            newSetValue = ColorOffsetSetValues[index].Correction + ColorOffsetCorrectionValues[index].Correction;
                            ColorOffsetSetValues[index] = new DotsCorrectionValue { Correction = newSetValue };
                            ColorOffsetCorrectionValues[index] = new DotsCorrectionValue { Correction = 0.0 };
                            RxGlobals.PrintSystem.ColorOffset[inkSupplyNumber] = Helpers.UnitConversion.ConvertUMToMM(ColorOffsetSetValues[index].Correction); // RxGlobals.PrintSystem.ColorOffset is in mm (???)
                        }
                        else
                        {
                            Console.WriteLine("Failed to set ColorOffset vcorrection value for ink supply #" + inkSupplyNumber.ToString());
                        }
                    }

                }
            }
        }

        public void ResetCorrectionValues()
        {
            AngleCorrectionValues.Clear();
            StitchCorrectionValues.Clear();
            RegisterCorrectionValues.Clear();
            BackwardRegisterCorrectionValues.Clear();
            ColorOffsetCorrectionValues.Clear();

            for (int i = 0; i < Alignment.kHeadsPerCluster; i++)
            {
                AngleCorrectionValues.Add(new AngleCorrectionValue() { Correction = 0.0 });
                StitchCorrectionValues.Add(new StitchCorrectionValue() { Correction = 0.0 });
                RegisterCorrectionValues.Add(new DotsCorrectionValue() { Correction = 0.0 });
                BackwardRegisterCorrectionValues.Add(new DotsCorrectionValue() { Correction = 0.0 });
                ColorOffsetCorrectionValues.Add(new DotsCorrectionValue() { Correction = 0.0 });
                ShowColorOffset.Add(new BooleanValue() { Value = false });
            }
        }

        public bool _IsLastPrintHeadOfInkSupply(int globalHeadNumber)
        {
            int totalNumberOfHeads = RxGlobals.PrintSystem.HeadsPerColor * RxGlobals.PrintSystem.ColorCnt;
            int headsPerColor = RxGlobals.PrintSystem.HeadsPerColor;
            if (globalHeadNumber < 0 || globalHeadNumber >= totalNumberOfHeads) return false;
            return ((globalHeadNumber % headsPerColor) == (headsPerColor - 1));
        }

        private bool _IsFirstPrintHeadOfInkSupply(int globalHeadNumber)
        {
            if (globalHeadNumber < 0 || globalHeadNumber >= RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.HeadsPerColor) return false;
            return ((globalHeadNumber % RxGlobals.PrintSystem.HeadsPerColor) == 0);
        }
    }
}
