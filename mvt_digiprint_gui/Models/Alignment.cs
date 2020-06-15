using Microsoft.Win32;
using RX_DigiPrint.Helpers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows;

namespace RX_DigiPrint.Models
{
    public class Alignment : INotifyPropertyChanged
    {
        public const int kMaxColors = 16; // max number of ink supplies
        public const int kHeadsPerCluster = 4;

        public int NumberOfClusters { get; set; }

        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        private bool _correctionValuesChanged;
        public bool CorrectionValuesChanged
        {
            get { return _correctionValuesChanged; }
            set { _correctionValuesChanged = value; OnPropertyChanged("CorrectionValuesChanged"); }
        }

        public bool ScanCheckValuesAvailable { get; set; }

        private bool _colorOffsetCorrectionUnitInDots;
        public bool ColorOffsetCorrectionUnitInDots
        {
            get { return _colorOffsetCorrectionUnitInDots; }
            set
            {
                _colorOffsetCorrectionUnitInDots = value;
                OnPropertyChanged("ColorOffsetCorrectionUnitInDots");
                UpdateUnits();
            }
        }

        private bool _angleCorrectionUnitInUM;
        public bool AngleStitchCorrectionsUnitIsInUM
        {
            get { return _angleCorrectionUnitInUM; }
            set
            {
                _angleCorrectionUnitInUM = value;
                OnPropertyChanged("AngleStitchCorrectionsUnitIsInUM");
                UpdateUnits();
            }
        }

        private bool _registerCorrectionUnitInDots;
        public bool RegisterCorrectionUnitInDots
        {
            get { return _registerCorrectionUnitInDots; }
            set { _registerCorrectionUnitInDots = value; OnPropertyChanged("RegisterCorrectionUnitInDots"); UpdateUnits(); }
        }

        public Dictionary<int, ClusterAlignment> ClusterAlignmentDictionary; // cluster no <--> InkSupplyAlignment

        private Helpers.ObservableCollectionEx<CorrectionValue> _angleCorrectionValues;
        public Helpers.ObservableCollectionEx<CorrectionValue> AngleCorrectionValues
        {
            get { return _angleCorrectionValues; }
            set
            {
                _angleCorrectionValues = value;
                OnPropertyChanged("AngleCorrectionValues");
            }
        }

        private Helpers.ObservableCollectionEx<CorrectionValue> _stitchCorrectionValues;
        public Helpers.ObservableCollectionEx<CorrectionValue> StitchCorrectionValues
        {
            get { return _stitchCorrectionValues; }
            set { _stitchCorrectionValues = value; OnPropertyChanged("StitchCorrectionValues"); }
        }

        public bool RobotIsConnected { get; set; }

        public Alignment()
        {
            ClusterAlignmentDictionary = new Dictionary<int, ClusterAlignment>();
            NumberOfClusters = 0;
            AngleCorrectionValues = new Helpers.ObservableCollectionEx<CorrectionValue>();
            StitchCorrectionValues = new Helpers.ObservableCollectionEx<CorrectionValue>();

            ColorOffsetCorrectionUnitInDots = false; // standard: in um
            AngleStitchCorrectionsUnitIsInUM = false; // standard: in revolutions
            RegisterCorrectionUnitInDots = false; // standard: in um
            RobotIsConnected = false;
            RxGlobals.PrintSystem.PropertyChanged += _PrintSystem_PropertyChanged;
        }

        public void _PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (RxGlobals.PrintSystem.ColorCnt == 0 || RxGlobals.PrintSystem.InkCylindersPerColor == 0
                 || RxGlobals.PrintSystem.HeadsPerColor == 0)
            {
                return;
            }

            if (!(e.PropertyName == "CorrectionValuesSet" || e.PropertyName.Equals("ColorCnt") || e.PropertyName.Equals("InkCylindersPerColor")
                || e.PropertyName == "HeadsPerColor"))
            {
                return;
            }



            NumberOfClusters = (RxGlobals.PrintSystem.HeadsPerColor * RxGlobals.PrintSystem.ColorCnt + 3) / 4;
            for (int c = ClusterAlignmentDictionary.Count; c < NumberOfClusters; c++)
            {
                ClusterAlignment clusterAlignment = new ClusterAlignment(c);
                clusterAlignment.IsScanning = (RxGlobals.PrintSystem.PrinterType == Services.EPrinterType.printer_TX801 || RxGlobals.PrintSystem.PrinterType == Services.EPrinterType.printer_TX801);
                ClusterAlignmentDictionary[c] = clusterAlignment;
            }
            foreach (var elem in ClusterAlignmentDictionary)
            {
                elem.Value.Init();
            }

            int headCnt = RxGlobals.PrintSystem.HeadsPerColor; // total number of heads
            AngleCorrectionValues.Clear();
            StitchCorrectionValues.Clear();
            for (int i = 0; i < headCnt; i++) AngleCorrectionValues.Add(new CorrectionValue() { Correction = 0.0 });
            for (int i = 0; i < headCnt; i++) StitchCorrectionValues.Add(new CorrectionValue() { Correction = 0.0 });
        }

        public void UpdateUnits()
        {
            foreach (var elem in ClusterAlignmentDictionary)
            {
                elem.Value.AngleStitchCorrectionsUnitIsInUM = AngleStitchCorrectionsUnitIsInUM;
                elem.Value.ColorOffsetCorrectionUnitInDots = ColorOffsetCorrectionUnitInDots;
                elem.Value.RegisterCorrectionUnitInDots = RegisterCorrectionUnitInDots;
            }
            ReloadClusters();
        }

        private void ReloadClusters()
        {
            foreach (var elem in ClusterAlignmentDictionary)
            {
                for (int i = 0; i < elem.Value.ColorOffsetCorrectionValues.Count; i++)
                {
                    var tmp = elem.Value.ColorOffsetCorrectionValues[i].Correction;
                    elem.Value.ColorOffsetCorrectionValues[i] = new DotsCorrectionValue() { Correction = tmp };
                }
            }
        }


        public void ImportScanCheckValues()
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();

            if (openFileDialog.ShowDialog() == true)
            {
                bool checkChanged = true;
                bool importedValues = false;
                var corrections = new List<ScanCheckImport.AlignmentCorrectionValues>();
                string errorMessage = "";
                bool result = false;
                try
                {
                    result = ScanCheckImport.ImportManager.GetCorrectionValues(
                        openFileDialog.FileName,
                        ref corrections,
                        ref errorMessage);
                }
                catch (Exception ex)
                {
                    RX_Common.MvtMessageBox.Information("Scan Check Import", "Failed to import values, check file format.\nError Message: " + ex.Message);
                    return;
                }

                if (result == true)
                {
                    for (int c = 0; c < NumberOfClusters; c++)
                    {
                        for (int h = 0; h < kHeadsPerCluster; h++)
                        {
                            int globalHeadNumber    = c * kHeadsPerCluster + h;
                            int color               = globalHeadNumber / RxGlobals.PrintSystem.HeadsPerColor;
                            int inkSupplyHeadNumber = globalHeadNumber % RxGlobals.PrintSystem.InkCylindersPerColor; // local head number in ink supply
                            int inkSupplyNumber     = color*RxGlobals.PrintSystem.InkCylindersPerColor + (0) ; // globalHeadNumber / RxGlobals.PrintSystem.PrintHeadsPerInkCylinderCnt; // global ink supply number

                            ScanCheckImport.AlignmentCorrectionValues correctionsForInkSupply = null;

                            if (!ClusterAlignmentDictionary.ContainsKey(c))
                            {
                                // cluster not available!!! programming error!
                                MessageBox.Show("Failed to update correction values.");
                            }

                            try { correctionsForInkSupply = corrections.Find(x => x.InkSupplyNumber == inkSupplyNumber); }
                            catch
                            {
                                // No values available for this ink supply. Try for next head. 
                                // Todo: Show error message?
                                continue;
                            }

                            double angleCorrection = 0.0;
                            double stitchCorrection = 0.0;
                            double registerCorrection = 0.0;
                            double backwardRegisterCorrection = 0.0;
                            double colorOffset = 0.0;

                            // Attention!! Units:

                            // VALUE         |   SCANCHECK FILE    |  main_ctrl   |  DIGI PRINT GUI
                            //---------------|---------------------|--------------|---------------------------------
                            // angle         |   revolutions       |  um          |  mm or revolutions
                            // stitch        |   revolutions       |  um          |  mm or revolutions
                            // register      |   um                |  um          |  mm or dots
                            // colorOffset   |   um                |  um          |  mm
                            // --> Convert all values to um for calculations

                            if (correctionsForInkSupply == null)
                            {
                                // No values available for this ink supply. Try for next head. 
                                // Todo: Show error message?
                                continue;
                            }

                            try
                            {
                                angleCorrection = Helpers.UnitConversion.ConvertRevolutionsToUm(Helpers.UnitConversion.CorrectionType.AngleCorrection, correctionsForInkSupply.AngleCorrection[inkSupplyHeadNumber]);
                                stitchCorrection = Helpers.UnitConversion.ConvertRevolutionsToUm(Helpers.UnitConversion.CorrectionType.StitchCorrection, correctionsForInkSupply.StitchCorrection[inkSupplyHeadNumber]);
                                registerCorrection = correctionsForInkSupply.RegisterCorrection[inkSupplyHeadNumber]; // already in um
                                colorOffset = correctionsForInkSupply.ColorOffsetCorrection; // already in um
                            }
                            catch (Exception ex)
                            {
                                // failed to read values
                                Console.WriteLine(ex.Message);
                            }

                            if (RxGlobals.PrintSystem.PrinterType == Services.EPrinterType.printer_TX801 ||
                                RxGlobals.PrintSystem.PrinterType == Services.EPrinterType.printer_TX802)
                            {
                                try
                                {
                                    backwardRegisterCorrection = correctionsForInkSupply.BidirectionalCorrection[inkSupplyHeadNumber]; // already in um
                                }
                                catch (Exception ex)
                                {
                                    Console.WriteLine(ex.Message);
                                }
                            }

                            if (checkChanged)
                            {
                                if (ClusterAlignmentDictionary[c].AngleCorrectionValues[h].Correction != angleCorrection
                                    || ClusterAlignmentDictionary[c].StitchCorrectionValues[h].Correction != stitchCorrection
                                    || ClusterAlignmentDictionary[c].RegisterCorrectionValues[h].Correction != registerCorrection
                                    || ClusterAlignmentDictionary[c].StitchCorrectionValues[h].Correction != backwardRegisterCorrection)
                                {
                                    checkChanged = false;
                                    RxGlobals.Alignment.CorrectionValuesChanged = true;
                                }
                            }
                            ClusterAlignmentDictionary[c].AngleCorrectionValues[h] = new AngleCorrectionValue { Correction = angleCorrection };
                            ClusterAlignmentDictionary[c].StitchCorrectionValues[h] = new StitchCorrectionValue { Correction = stitchCorrection };
                            ClusterAlignmentDictionary[c].RegisterCorrectionValues[h] = new DotsCorrectionValue { Correction = registerCorrection };
                            ClusterAlignmentDictionary[c].BackwardRegisterCorrectionValues[h] = new DotsCorrectionValue { Correction = backwardRegisterCorrection };

                            //if (ClusterAlignmentDictionary[c]._IsLastPrintHeadOfInkSupply(globalHeadNumber) == true)
                            //{
                            ClusterAlignmentDictionary[c].ColorOffsetCorrectionValues[h] = new DotsCorrectionValue { Correction = colorOffset };
                            //}

                            importedValues = true;
                        }
                    }
                }
                else
                {
                    MessageBox.Show("Failed to import correction values.");
                }
                ScanCheckValuesAvailable = importedValues;
                foreach (var elem in ClusterAlignmentDictionary)
                {
                    elem.Value.ScanCheckValuesAvailable = ScanCheckValuesAvailable;
                }
            }
        }

        public void ClearCorrectionValues()
        {
            foreach (var elem in ClusterAlignmentDictionary)
            {
                elem.Value.ResetCorrectionValues();
            }
            ScanCheckValuesAvailable = false;
            foreach (var elem in ClusterAlignmentDictionary)
            {
                elem.Value.ScanCheckValuesAvailable = ScanCheckValuesAvailable;
            }
        }

        public void UpdateRegisterCorrectionValues(int clusterNumber, int globalHeadNumber, double difference)
        {
            // unit:
            double differenceInUm = difference;
            if (RegisterCorrectionUnitInDots)
            {
                differenceInUm = Helpers.UnitConversion.ConvertDotsToUM(1200, difference);
            }

            int inkSupplyNumber = 0; //globalHeadNumber / RxGlobals.PrintSystem.PrintHeadsPerInkCylinderCnt;
            int nextHeadNumber  = 0; // globalHeadNumber;
            int totalNumberOfHeads = RxGlobals.PrintSystem.HeadsPerColor;
            while (nextHeadNumber < totalNumberOfHeads)
            {
                int thisInkSupplyNumber = 0; //nextHeadNumber / RxGlobals.PrintSystem.PrintHeadsPerInkCylinderCnt;
                if (thisInkSupplyNumber != inkSupplyNumber)
                {
                    break;
                }
                else
                {
                    int thisClusterNumber = -1;
                    if (RxGlobals.PrintSystem.IsScanning)
                    {
                        int remainder = (4 - ((RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.HeadsPerColor) % 4)) % 4;
                        thisClusterNumber = (nextHeadNumber + remainder) / 4;
                    }
                    else
                    {
                        thisClusterNumber = nextHeadNumber / 4;
                    }
                    try
                    {
                        int h = nextHeadNumber % 4;
                        double correction = ClusterAlignmentDictionary[thisClusterNumber].RegisterCorrectionValues[h].Correction + differenceInUm;
                        ClusterAlignmentDictionary[thisClusterNumber].RegisterCorrectionValues[h] = new DotsCorrectionValue { Correction = correction };
                    }
                    catch { }

                    nextHeadNumber++;
                }
            }
        }
    }

    public class BooleanValue : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private bool _value;
        public bool Value
        {
            get
            {
                return _value;
            }
            set
            {
                _value = value;
                OnPropertyChanged("Value");
            }
        }

        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }
    }

    public class ClusterAlignment : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        public int ClusterNumber { get; set; } // 0, 1, ...
        public int ClusterDisplyNumber { get; set; } // 1, 2, ...
        private string _ClusterName;
        public string ClusterName
        {
            get { return _ClusterName; }
            set { _ClusterName = value; OnPropertyChanged("ClusterName"); }
        }

        private bool _isScanning { get; set; }
        public bool IsScanning
        {
            get { return _isScanning; }
            set
            {
                _isScanning = value;
                OnPropertyChanged("IsScanning");
            }
        }

        private bool _robotIsConnected { get; set; }
        public bool RobotIsConnected
        {
            get { return _robotIsConnected; }
            set
            {
                _robotIsConnected = value;
                OnPropertyChanged("RobotIsConnected");
                OnPropertyChanged("InfoButtonsEnabled");
            }
        }

        private bool _scanCheckValuesAvailable { get; set; }
        public bool ScanCheckValuesAvailable
        {
            get { return _scanCheckValuesAvailable; }
            set
            {
                _scanCheckValuesAvailable = value;
                OnPropertyChanged("ScanCheckValuesAvailable");
                OnPropertyChanged("InfoButtonsEnabled");
            }
        }

        private bool _infoButtonsEnabled { get; set; }
        public bool InfoButtonsEnabled
        {
            get { return (!(RobotIsConnected && ScanCheckValuesAvailable)); }
        }

        private bool _isFirstInView { get; set; }
        public bool IsFirstInView
        {
            get { return _isFirstInView; }
            set
            {
                _isFirstInView = value;
                OnPropertyChanged("IsFirstInView");
            }
        }

        private bool _colorOffsetCorrectionUnitInDots;
        public bool ColorOffsetCorrectionUnitInDots
        {
            get { return _colorOffsetCorrectionUnitInDots; }
            set
            {
                _colorOffsetCorrectionUnitInDots = value;
                OnPropertyChanged("ColorOffsetCorrectionUnitInDots");
            }
        }

        private bool _angleCorrectionUnitInUM;
        public bool AngleStitchCorrectionsUnitIsInUM
        {
            get { return _angleCorrectionUnitInUM; }
            set
            {
                _angleCorrectionUnitInUM = value;
                OnPropertyChanged("AngleStitchCorrectionsUnitIsInUM");
            }
        }

        private bool _registerCorrectionUnit;
        public bool RegisterCorrectionUnitInDots
        {
            get { return _registerCorrectionUnit; }
            set
            {
                _registerCorrectionUnit = value;
                OnPropertyChanged("RegisterCorrectionUnitInDots");
            }
        }

        private bool _hasVisibleColorOffsetCorrectionValue { get; set; }
        public bool HasVisibleColorOffsetCorrectionValue
        {
            get { return _hasVisibleColorOffsetCorrectionValue; }
            set
            {
                _hasVisibleColorOffsetCorrectionValue = value;
                OnPropertyChanged("HasVisibleColorOffsetCorrectionValue");
            }
        }

        private Helpers.ObservableCollectionEx<AngleCorrectionValue> _angleCorrectionValues;
        public Helpers.ObservableCollectionEx<AngleCorrectionValue> AngleCorrectionValues
        {
            get { return _angleCorrectionValues; }
            set { _angleCorrectionValues = value; OnPropertyChanged("AngleCorrectionValues"); }
        }

        private Helpers.ObservableCollectionEx<DotsCorrectionValue> _registerCorrectionValues;
        public Helpers.ObservableCollectionEx<DotsCorrectionValue> RegisterCorrectionValues
        {
            get { return _registerCorrectionValues; }
            set { _registerCorrectionValues = value; OnPropertyChanged("RegisterCorrectionValues"); }
        }
        private Helpers.ObservableCollectionEx<DotsCorrectionValue> _backwardRegisterCorrectionValues;
        public Helpers.ObservableCollectionEx<DotsCorrectionValue> BackwardRegisterCorrectionValues
        {
            get { return _backwardRegisterCorrectionValues; }
            set { _backwardRegisterCorrectionValues = value; OnPropertyChanged("BackwardRegisterCorrectionValues"); }
        }

        private Helpers.ObservableCollectionEx<StitchCorrectionValue> _stitchCorrectionValues;
        public Helpers.ObservableCollectionEx<StitchCorrectionValue> StitchCorrectionValues
        {
            get { return _stitchCorrectionValues; }
            set { _stitchCorrectionValues = value; OnPropertyChanged("StitchCorrectionValues"); }
        }

        private Helpers.ObservableCollectionEx<DotsCorrectionValue> _colorOffsetCorrectionValues;
        public Helpers.ObservableCollectionEx<DotsCorrectionValue> ColorOffsetCorrectionValues
        {
            get { return _colorOffsetCorrectionValues; }
            set { _colorOffsetCorrectionValues = value; OnPropertyChanged("ColorOffsetCorrectionValues"); }
        }

        private Helpers.ObservableCollectionEx<DotsCorrectionValue> _colorOffsetSetValues;
        public Helpers.ObservableCollectionEx<DotsCorrectionValue> ColorOffsetSetValues
        {
            get { return _colorOffsetSetValues; }
            set { _colorOffsetSetValues = value; OnPropertyChanged("ColorOffsetCorrectionValues"); }
        }

        private Helpers.ObservableCollectionEx<DotsCorrectionValue> _registerSetValues;
        public Helpers.ObservableCollectionEx<DotsCorrectionValue> RegisterSetValues
        {
            get { return _registerSetValues; }
            set { _registerSetValues = value; OnPropertyChanged("RegisterSetValues"); }
        }

        private Helpers.ObservableCollectionEx<DotsCorrectionValue> _backwardRegisterSetValues;
        public Helpers.ObservableCollectionEx<DotsCorrectionValue> BackwardRegisterSetValues
        {
            get { return _backwardRegisterSetValues; }
            set { _backwardRegisterSetValues = value; OnPropertyChanged("BackwardRegisterSetValues"); }
        }



        private Helpers.ObservableCollectionEx<BooleanValue> _showColorOffset;
        public Helpers.ObservableCollectionEx<BooleanValue> ShowColorOffset
        {
            get { return _showColorOffset; }
            set { _showColorOffset = value; OnPropertyChanged("ShowColorOffset"); }
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
            IsScanning = RxGlobals.PrintSystem.IsScanning;
        }

        public void Init()
        {
            ColorOffsetCorrectionUnitInDots = RxGlobals.Alignment.ColorOffsetCorrectionUnitInDots;
            AngleStitchCorrectionsUnitIsInUM = RxGlobals.Alignment.AngleStitchCorrectionsUnitIsInUM;
            RegisterCorrectionUnitInDots = RxGlobals.Alignment.RegisterCorrectionUnitInDots;

            InitCorrectionValues();
            RobotIsConnected = RxGlobals.Alignment.RobotIsConnected;
            ScanCheckValuesAvailable = RxGlobals.Alignment.ScanCheckValuesAvailable;
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
                    int remainder = (4 - ((RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.HeadsPerColor) % 4)) % 4;
                    globalHeadNumber = ClusterNumber * Alignment.kHeadsPerCluster + h - remainder;
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
                    if (IsScanning)
                    {
                        showColorOffset = _IsLastPrintHeadOfInkSupply(globalHeadNumber);
                    }
                    else
                    {
                        showColorOffset = _IsLastPrintHeadOfInkSupply(globalHeadNumber);
                    }
                    ShowColorOffset.Add(new BooleanValue() { Value = showColorOffset });

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
                }
            }

            if (IsScanning)
            {
                AngleCorrectionValues.ReverseOrder();
                StitchCorrectionValues.ReverseOrder();
                RegisterCorrectionValues.ReverseOrder();
                BackwardRegisterCorrectionValues.ReverseOrder();
                ColorOffsetCorrectionValues.ReverseOrder();
                ColorOffsetSetValues.ReverseOrder();
                RegisterSetValues.ReverseOrder();
                BackwardRegisterSetValues.ReverseOrder();
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
                    int remainder = (4 - ((RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.HeadsPerColor) % 4)) % 4;
                    globalHeadNumber = ClusterNumber * Alignment.kHeadsPerCluster + h - remainder;
                }
                else
                {
                    globalHeadNumber = ClusterNumber * Alignment.kHeadsPerCluster + h;
                }
                if (globalHeadNumber >= 0)
                {
                    int inkSupplyNumber = 0; // globalHeadNumber / RxGlobals.PrintSystem.PrintHeadsPerInkCylinderCnt;
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
                    if (IsScanning)
                    {
                        showColorOffset = _IsLastPrintHeadOfInkSupply(globalHeadNumber);
                    }
                    else
                    {
                        showColorOffset = _IsLastPrintHeadOfInkSupply(globalHeadNumber);
                    }
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
            int totoalNumberOfHeads = RxGlobals.PrintSystem.HeadsPerColor;
            int headsPerColor = RxGlobals.PrintSystem.HeadsPerColor;
            if (globalHeadNumber < 0 || globalHeadNumber >= totoalNumberOfHeads) return false;
            return ((globalHeadNumber % headsPerColor) == (headsPerColor - 1));
        }

        private bool _IsFirstPrintHeadOfInkSupply(int globalHeadNumber)
        {
            if (globalHeadNumber < 0 || globalHeadNumber >= RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.HeadsPerColor) return false;
            return ((globalHeadNumber % RxGlobals.PrintSystem.HeadsPerColor) == 0);
        }
    }
}
