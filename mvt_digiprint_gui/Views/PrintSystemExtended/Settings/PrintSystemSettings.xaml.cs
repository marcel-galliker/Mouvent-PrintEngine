using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.PrintSystemView;
using System.ComponentModel;
using System.Windows;

namespace RX_DigiPrint.Views.PrintSystemExtendedView
{
    /// <summary>
    /// Interaction logic for PrintSystemSettings.xaml
    /// </summary>
    public partial class PrintSystemSettings : CustomWindow, INotifyPropertyChanged
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

        // Print System:
        private int _colorCnt;
        public int LocalColorCnt
        {
            get { return _colorCnt; }
            set
            {
                _colorCnt = value;
                OnPropertyChanged("ColorCnt");
            }
        }

        private int _inkSupplyPerColorCnt;
        public int LocalInkSupplyPerColorCnt
        {
            get { return _inkSupplyPerColorCnt; }
            set
            {
                _inkSupplyPerColorCnt = value;
                OnPropertyChanged("InkSupplyPerColorCnt");
            }
        }

        private int _printHeadsPerColor;
        public int LocalPrintHeadsPerColor
        {
            get { return _printHeadsPerColor; }
            set
            {
                _printHeadsPerColor = value;
                OnPropertyChanged("PrintHeadsPerInkSupplyCnt");
            }
        }

        public EN_PrinterTypeList LocalPrinterTypeList { get; set; }

        private EPrinterType _localPrinterType;
        public EPrinterType LocalPrinterType
        {
            get { return _localPrinterType; }
            set { _localPrinterType = value; OnPropertyChanged("LocalPrinterType"); }
        }

        private OffsetValues _OffsetValues;
        public OffsetValues OffsetValues
        {
            get
            {
                return _OffsetValues;
            }
            set
            {
                _OffsetValues = value; OnPropertyChanged("OffsetValues");

            }
        }

        private EncoderValues _EncoderValues0;
        public EncoderValues EncoderValues0
        {
            get
            {
                return _EncoderValues0;
            }
            set
            {
                _EncoderValues0 = value; OnPropertyChanged("EncoderValues0");

            }
        }

        private EncoderValues _EncoderValues1;
        public EncoderValues EncoderValues1
        {
            get
            {
                return _EncoderValues1;
            }
            set
            {
                _EncoderValues1 = value; OnPropertyChanged("EncoderValues1");

            }
        }

        private StepperValues _StepperValues;
        public StepperValues StepperValues
        {
            get
            {
                return _StepperValues;
            }
            set
            {
                _StepperValues = value; OnPropertyChanged("StepperValues");

            }
        }

        private EncoderView EncoderView { get; set; }

        private OffsetsGrid OffsetsGrid { get; set; }

        private StepperGrid StandardStepperGrid { get; set; }
        private StepperGridCleaf StepperGridCleaf { get; set; }
        private StepperGridDP803 StepperGridDP803 { get; set; }

        private StepperGridLB702 StepperGridLB702 { get; set; }


        public PrintSystemSettings()
        {
            InitializeComponent();
            DataContext = this;

            LocalPrinterType = RxGlobals.PrintSystem.PrinterType;
            LocalColorCnt = RxGlobals.PrintSystem.ColorCnt;
            LocalInkSupplyPerColorCnt = RxGlobals.PrintSystem.InkCylindersPerColor;
            LocalPrintHeadsPerColor   = RxGlobals.PrintSystem.HeadsPerColor;
            LocalPrinterTypeList = new EN_PrinterTypeList();

            OffsetValues = new OffsetValues();
            OffsetsGrid = new OffsetsGrid();
            OffsetsGrid.Margin = new Thickness(10, 0, 0, 0);
            OffsetsGrid.SetDataContext(OffsetValues);
            OffsetsGrid.UpdateVisibility(LocalPrinterType);

            TopPanel.Children.Add(OffsetsGrid);
            for (int i = 0; i < RxGlobals.Encoder.Length; i++)
                if (RxGlobals.Encoder[i] == null) RxGlobals.Encoder[i] = new RX_DigiPrint.Models.Encoder(i);

            EncoderValues0 = new EncoderValues(0);
            EncoderValues1 = new EncoderValues(1);
            EncoderView = new EncoderView();
            EncoderView.SetDataContext(EncoderValues0, EncoderValues1);
            EncoderView.UpdateVisibility(LocalPrinterType);
            EncoderView.Margin
                 = new Thickness(10, 0, 0, 0);
            TopPanel.Children.Add(EncoderView);

            StepperValues = new StepperValues();
            StandardStepperGrid = null;
            StepperGridCleaf = null;
            StepperGridDP803 = null;
            StepperGridLB702 = null;
            UpdateStepperSettingsView();
        }

        private void UpdateView()
        {
            OffsetsGrid.UpdateVisibility(LocalPrinterType);
            UpdateStepperSettingsView();
        }

        private void UpdateStepperSettingsView()
        {
            StepperGrid.Children.Clear();
            if (StandardStepperGrid != null) StandardStepperGrid = null;
            if (StepperGridCleaf != null) StepperGridCleaf = null;
            if (StepperGridDP803 != null) StepperGridDP803 = null;
            if (StepperGridLB702 != null) StepperGridLB702 = null;
            
            switch (LocalPrinterType)
            {
                case EPrinterType.printer_cleaf:
                    StepperGridCleaf = new StepperGridCleaf();
                    StepperGridCleaf.SetDataContext(StepperValues);
                    StepperGrid.Children.Add(StepperGridCleaf);
                    break;

                case EPrinterType.printer_DP803:
                    StepperGridDP803 = new StepperGridDP803();
                    StepperGridDP803.SetDataContext(StepperValues);
                    StepperGrid.Children.Add(StepperGridDP803);
                    break;

                case EPrinterType.printer_LB702_UV:
                case EPrinterType.printer_LB702_WB:
                case EPrinterType.printer_LH702:
                    StepperGridLB702 = new StepperGridLB702();
                    StepperGridLB702.SetDataContext(StepperValues);
                    StepperGridLB702.SetPrinterType(LocalPrinterType);
                    StepperGrid.Children.Add(StepperGridLB702);
                    break;

                case EPrinterType.printer_TX801:
                case EPrinterType.printer_TX802:
                case EPrinterType.printer_TX404:

                    break;

                default:
                    StandardStepperGrid = new StepperGrid();
                    StandardStepperGrid.SetDataContext(StepperValues);
                    StandardStepperGrid.UpdateVisibility(LocalPrinterType);
                    StepperGrid.Children.Add(StandardStepperGrid);
                    break;
            }
        }

        private void OkayButton_Click(object sender, RoutedEventArgs e)
        {
            RxGlobals.PrintSystem.ColorCnt            = LocalColorCnt;
            RxGlobals.PrintSystem.InkCylindersPerColor = LocalInkSupplyPerColorCnt;
            RxGlobals.PrintSystem.HeadsPerColor       = LocalPrintHeadsPerColor;
            RxGlobals.PrintSystem.PrinterType         = LocalPrinterType;

            SaveOffsetValues();
            SaveEncoderValues();
            SaveStepperValues();

            DialogResult = true;
        }

        private void SaveStepperValues()
        {
            bool changed = false;
            switch (RxGlobals.PrintSystem.PrinterType)
            {
                case EPrinterType.printer_test_slide:
                case EPrinterType.printer_test_slide_only:
                case EPrinterType.printer_test_slide_HB:
                case EPrinterType.printer_test_table:
                case EPrinterType.printer_Dropwatcher:
                    RxGlobals.Stepper.RefHeight = StepperValues.RefHeight;
                    RxGlobals.Stepper.WipeHeight = StepperValues.WipeHeight;
                    RxGlobals.Stepper.CapHeight = StepperValues.CapHeight;
                    RxGlobals.Stepper.AdjustPos = StepperValues.AdjustPos;
                    RxGlobals.PrintSystem.ManualFlightTimeComp = StepperValues.ManualFlightTimeComp;
                    break;
                case EPrinterType.printer_LB701:
                    RxGlobals.Stepper.RefHeight = StepperValues.RefHeight;
                    RxGlobals.PrintSystem.ManualFlightTimeComp = StepperValues.ManualFlightTimeComp;
                    break;
                case EPrinterType.printer_cleaf:
                    changed = false;
                    if (RxGlobals.Stepper.WipeHeight != StepperValues.WipeHeight) changed = true;
                    RxGlobals.Stepper.WipeHeight = StepperValues.WipeHeight;
                    if (RxGlobals.Stepper.CapHeight != StepperValues.CapHeight) changed = true;
                    RxGlobals.Stepper.CapHeight = StepperValues.CapHeight;
                    for (int i = 0; i < RxGlobals.Stepper.Robot.Length; i++)
                    {
                        if (RxGlobals.Stepper.Robot[i].ref_height != StepperValues.Robot[i].ref_height) changed = true;
                        RxGlobals.Stepper.Robot[i].ref_height = StepperValues.Robot[i].ref_height;
                        if (RxGlobals.Stepper.Robot[i].head_align != StepperValues.Robot[i].head_align) changed = true;
                        RxGlobals.Stepper.Robot[i].head_align = StepperValues.Robot[i].head_align;
                    }
                    if (changed)
                    {
                        RxGlobals.Stepper.Changed = true;
                    }
                    break;
                case EPrinterType.printer_DP803:
                    changed = false;
                    for (int i = 0; i < RxGlobals.Stepper.Robot.Length; i++)
                    {
                        if (RxGlobals.Stepper.Robot[i].ref_height != StepperValues.Robot[i].ref_height) changed = true;
                        RxGlobals.Stepper.Robot[i].ref_height = StepperValues.Robot[i].ref_height;
                        if (RxGlobals.Stepper.Robot[i].head_align != StepperValues.Robot[i].head_align) changed = true;
                        RxGlobals.Stepper.Robot[i].head_align = StepperValues.Robot[i].head_align;
                    }
                    if (changed)
                    {
                        RxGlobals.Stepper.Changed = true;
                    }
                    break;
                case EPrinterType.printer_LB702_UV:
                case EPrinterType.printer_LB702_WB:
                case EPrinterType.printer_LH702:
                    changed = false;
                    for (int i = 0; i < RxGlobals.Stepper.Robot.Length; i++)
                    {
                        if (RxGlobals.Stepper.Robot[i].ref_height_back != StepperValues.Robot[i].ref_height_back) changed = true;
                        RxGlobals.Stepper.Robot[i].ref_height_back = StepperValues.Robot[i].ref_height_back;
                        if (RxGlobals.Stepper.Robot[i].ref_height_front != StepperValues.Robot[i].ref_height_front) changed = true;
                        RxGlobals.Stepper.Robot[i].ref_height_front = StepperValues.Robot[i].ref_height_front;
                    }
                    RxGlobals.PrintSystem.ManualFlightTimeComp = StepperValues.ManualFlightTimeComp;
                    if (RxGlobals.StepperStatus[0].ClnUsed || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LB702_WB)
                    {
                        if (RxGlobals.Stepper.WipeDelay != StepperValues.WipeDelay) changed = true;
                        RxGlobals.Stepper.WipeDelay = StepperValues.WipeDelay;
                        if (RxGlobals.Stepper.WipeSpeed != StepperValues.WipeSpeed) changed = true;
                        RxGlobals.Stepper.WipeSpeed = StepperValues.WipeSpeed;
                        for (int i = 0; i < RxGlobals.Stepper.Robot.Length; i++)
                        {
                            if (RxGlobals.Stepper.Robot[i].cap_height != StepperValues.Robot[i].cap_height) changed = true;
                            RxGlobals.Stepper.Robot[i].cap_height = StepperValues.Robot[i].cap_height;
                        }
                    }
                    if (changed)
                    {
                        RxGlobals.Stepper.Changed = true;
                    }
                    break;
                default:
                    break;
            }
        }

        private void SaveEncoderValues()
        {
            if (RxGlobals.Encoder.Length > 0 && RxGlobals.Encoder[0] != null)
            {
                RxGlobals.Encoder[0].AmplNew = EncoderValues0.AmplNew;
                RxGlobals.Encoder[0].AmplOld = EncoderValues0.AmplOld;
                RxGlobals.Encoder[0].Meters = EncoderValues0.Meters;
            }
            if (EncoderView.HasEncoder2)
            {
                if (RxGlobals.Encoder.Length > 1 && RxGlobals.Encoder[1] != null)
                {
                    RxGlobals.Encoder[1].AmplNew = EncoderValues1.AmplNew;
                    RxGlobals.Encoder[1].AmplOld = EncoderValues1.AmplOld;
                    RxGlobals.Encoder[1].Meters = EncoderValues1.Meters;
                }
            }
        }

        private void SaveOffsetValues()
        {
            if (OffsetsGrid.Visibility == Visibility.Visible)
            {
                if (OffsetsGrid.Encoder.Visibility == Visibility.Visible) RxGlobals.PrintSystem.OffsetIncPerMeter = OffsetValues.OffsetIncPerMeter;
                if (OffsetsGrid.Angle.Visibility == Visibility.Visible) RxGlobals.PrintSystem.OffsetAngle = OffsetValues.OffsetAngle;
                if (OffsetsGrid.Belt.Visibility == Visibility.Visible) RxGlobals.PrintSystem.OffsetStep = OffsetValues.OffsetStep;
                if (OffsetsGrid.Overlap.Visibility == Visibility.Visible) RxGlobals.PrintSystem.Overlap = OffsetValues.Overlap;
                if (OffsetsGrid.EncoderVerso.Visibility == Visibility.Visible) RxGlobals.PrintSystem.OffsetIncPerMeterVerso = OffsetValues.OffsetIncPerMeterVerso;
                if (OffsetsGrid.DistVerso.Visibility == Visibility.Visible) RxGlobals.PrintSystem.OffsetVerso = OffsetValues.OffsetVerso;
                if (OffsetsGrid.ManualFlightTimeComp.Visibility == Visibility.Visible) RxGlobals.PrintSystem.ManualFlightTimeComp = OffsetValues.ManualFlightTimeComp;
            }
        }

        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }

        private void PrinterTypeCombo_LostFocus(object sender, RoutedEventArgs e)
        {
            UpdateView();
        }

        private void CustomWindow_Loaded(object sender, RoutedEventArgs e)
        {
            RxGlobals.Screen.PlaceWindow(this);
        }
    }
}
