using Microsoft.Win32;
using RX_Common;
using RX_DigiPrint.Helpers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows;

namespace RX_DigiPrint.Models
{
    public class Alignment : RxBindable
    {
        public const int kMaxColors = 16; // max number of ink supplies
        public const int kHeadsPerCluster = 4;

        public int NumberOfClusters { get; set; }

        private bool _colorOffsetEditMode;
        public bool ColorOffsetEditMode
        {
            get { return _colorOffsetEditMode; }
            set 
            {
                SetProperty(ref _colorOffsetEditMode, value);

                foreach (var elem in ClusterAlignmentDictionary)
                {
                    elem.Value.ColorOffsetEditMode = value;
                   
                }
            }
        }

        private bool _angleEditMode;
        public bool AngleEditMode
        {
            get { return _angleEditMode; }
            set
            {
                SetProperty(ref _angleEditMode, value);

                foreach (var elem in ClusterAlignmentDictionary)
                {
                    elem.Value.AngleEditMode = value;

                }
            }
        }

        private bool _hasStitchEditMode;
        public bool HasStitchEditMode
        {
            get { return _hasStitchEditMode; }
            set
            {
                SetProperty(ref _hasStitchEditMode, value);

                foreach (var elem in ClusterAlignmentDictionary)
                {
                    elem.Value.HasStitchEditMode = value;

                }
            }
        }


        private bool _stitchEditMode;
        public bool StitchEditMode
        {
            get { return _stitchEditMode; }
            set
            {
                SetProperty(ref _stitchEditMode, value);

                foreach (var elem in ClusterAlignmentDictionary)
                {
                    elem.Value.StitchEditMode = value;

                }
            }
        }

        private bool _registerEditMode;
        public bool RegisterEditMode
        {
            get { return _registerEditMode; }
            set
            {
                SetProperty(ref _registerEditMode, value);

                foreach (var elem in ClusterAlignmentDictionary)
                {
                    elem.Value.RegisterEditMode = value;

                }
            }
        }

        private bool _backwardsRegisterEditMode;
        public bool BackwardsRegisterEditMode
        {
            get { return _backwardsRegisterEditMode; }
            set
            {
                SetProperty(ref _backwardsRegisterEditMode, value);

                foreach (var elem in ClusterAlignmentDictionary)
                {
                    elem.Value.BackwardsRegisterEditMode = value;
                }
            }
        }

        private bool _applyCorrectionValuesToAllFollowingPrintHeads;
        public bool ApplyCorrectionValuesToAllFollowingPrintHeads
        {
            get { return _applyCorrectionValuesToAllFollowingPrintHeads; }
            set
            {
                SetProperty(ref _applyCorrectionValuesToAllFollowingPrintHeads, value);
            }
        }
        

        private bool _jetCompensationEditMode;
        public bool JetCompensationEditMode
        {
            get { return _jetCompensationEditMode; }
            set
            {
                SetProperty(ref _jetCompensationEditMode, value);

                foreach (var elem in ClusterAlignmentDictionary)
                {
                    elem.Value.JetCompensationEditMode = value;
                }
            }
        }

        private bool _correctionValuesChanged;
        public bool CorrectionValuesChanged
        {
            get { return _correctionValuesChanged; }
            set
            { SetProperty(ref _correctionValuesChanged, value);}
        }

        private bool _colorOffsetCorrectionUnitInDots;
        public bool ColorOffsetCorrectionUnitInDots
        {
            get { return _colorOffsetCorrectionUnitInDots; }
            set
            {
                SetProperty(ref _colorOffsetCorrectionUnitInDots, value);
                UpdateUnits();
            }
        }

        private bool _angleCorrectionUnitInRevolutions;
        public bool AngleStitchCorrectionsUnitInRevolutions
        {
            get { return _angleCorrectionUnitInRevolutions; }
            set
            {
                SetProperty(ref _angleCorrectionUnitInRevolutions, value);
                UpdateUnits();
            }
        }

        private bool _registerCorrectionUnitInDots;
        public bool RegisterCorrectionUnitInDots
        {
            get { return _registerCorrectionUnitInDots; }
            set 
            {
               SetProperty(ref _registerCorrectionUnitInDots, value);
               UpdateUnits(); 
            }
        }

        public Dictionary<int, ClusterAlignment> ClusterAlignmentDictionary; // cluster no <--> InkSupplyAlignment

        private Helpers.ObservableCollectionEx<CorrectionValue> _angleCorrectionValues;
        public Helpers.ObservableCollectionEx<CorrectionValue> AngleCorrectionValues
        {
            get { return _angleCorrectionValues; }
            set
            {
                SetProperty(ref _angleCorrectionValues, value);
            }
        }

        private Helpers.ObservableCollectionEx<CorrectionValue> _stitchCorrectionValues;
        public Helpers.ObservableCollectionEx<CorrectionValue> StitchCorrectionValues
        {
            get { return _stitchCorrectionValues; }
            set { SetProperty(ref _stitchCorrectionValues, value); }
        }

        public bool RobotIsConnected { get; set; }

        public Alignment()
        {
            ClusterAlignmentDictionary = new Dictionary<int, ClusterAlignment>();
            NumberOfClusters = 0;
            AngleCorrectionValues = new Helpers.ObservableCollectionEx<CorrectionValue>();
            StitchCorrectionValues = new Helpers.ObservableCollectionEx<CorrectionValue>();

            ColorOffsetCorrectionUnitInDots = false; // standard: in mm
            AngleStitchCorrectionsUnitInRevolutions = false; // standard: in revolutions
            RegisterCorrectionUnitInDots = false; // standard: in mm
            RobotIsConnected = false;
            RxGlobals.PrintSystem.PropertyChanged += _PrintSystem_PropertyChanged;
            AngleEditMode = false;
            ColorOffsetEditMode = false;
            StitchEditMode = false;
            HasStitchEditMode = true;
            RegisterEditMode = false;
            BackwardsRegisterEditMode = false;
            JetCompensationEditMode = false;
            ApplyCorrectionValuesToAllFollowingPrintHeads = false;
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

            HasStitchEditMode = RxGlobals.PrintSystem.HeadsPerColor == 1 ? false : true;

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

            int headCnt = RxGlobals.PrintSystem.HeadsPerColor * RxGlobals.PrintSystem.ColorCnt; // total number of heads
            AngleCorrectionValues.Clear();
            StitchCorrectionValues.Clear();
            for (int i = 0; i < headCnt; i++) AngleCorrectionValues.Add(new CorrectionValue() { Correction = 0.0 });
            for (int i = 0; i < headCnt; i++) StitchCorrectionValues.Add(new CorrectionValue() { Correction = 0.0 });
        }

        public void UpdateUnits()
        {
            foreach (var elem in ClusterAlignmentDictionary)
            {
                elem.Value.AngleStitchCorrectionsUnitInRevolutions = AngleStitchCorrectionsUnitInRevolutions;
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
                            int globalHeadNumber = c * kHeadsPerCluster + h;
                            int inkSupplyNumber = globalHeadNumber / RxGlobals.PrintSystem.HeadsPerInkCylinder; // global ink supply number
                            int inkSupplyHeadNumber = globalHeadNumber % RxGlobals.PrintSystem.HeadsPerInkCylinder; // local head number in ink supply

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
                        }
                    }
                }
                else
                {
                    MessageBox.Show("Failed to import correction values.");
                }
            }
        }

        public void ClearCorrectionValues()
        {
            foreach (var elem in ClusterAlignmentDictionary)
            {
                elem.Value.ResetCorrectionValues();
            }
        }

        public void UpdateRegisterCorrectionValues(int clusterNumber, int globalHeadNumber, double difference)
        {
            // difference must be given in um!!!

            int inkSupplyNumber = globalHeadNumber / RxGlobals.PrintSystem.HeadsPerInkCylinder;
            int nextHeadNumber  =  globalHeadNumber;
            int totalNumberOfHeads = RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.HeadsPerColor;
            while (nextHeadNumber < totalNumberOfHeads)
            {
                int thisInkSupplyNumber =  nextHeadNumber / RxGlobals.PrintSystem.HeadsPerInkCylinder;
                if (thisInkSupplyNumber != inkSupplyNumber)
                {
                    break;
                }
                else
                {
                    int thisClusterNumber = -1;
                    
                    if (RxGlobals.PrintSystem.IsScanning)
                    {
                        // int remainder = (4 - ((RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.HeadsPerColor) % 4)) % 4;
                        thisClusterNumber = nextHeadNumber / 4;
                    }
                    else
                    {
                        thisClusterNumber = nextHeadNumber / 4;
                    }
                    try
                    {
                        int h = nextHeadNumber % 4;
                        if (RxGlobals.PrintSystem.IsScanning)
                        {
                            h = (3 - h);
                        }
                        double correction = ClusterAlignmentDictionary[thisClusterNumber].RegisterCorrectionValues[h].Correction + difference;
                        ClusterAlignmentDictionary[thisClusterNumber].RegisterCorrectionValues[h] = new DotsCorrectionValue { Correction = correction };
                    }
                    catch { }

                    nextHeadNumber++;

                    if (!ApplyCorrectionValuesToAllFollowingPrintHeads)
                    {
                        break;
                    }
                }
            }
        }
    }

    public class BooleanValue : RxBindable
    {
        
        private bool _value;
        public bool Value
        {
            get
            {
                return _value;
            }
            set
            {
                SetProperty(ref _value, value);
            }
        }
    }
}
