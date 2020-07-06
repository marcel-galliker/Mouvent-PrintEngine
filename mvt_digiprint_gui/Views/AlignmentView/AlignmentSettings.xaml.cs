using RX_Common;
using RX_DigiPrint.Models;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows;

namespace RX_DigiPrint.Views.AlignmentView
{
    /// <summary>
    /// Interaction logic for AlignmentSettings.xaml
    /// </summary>
    public partial class AlignmentSettings : Window, INotifyPropertyChanged
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

        const int enumValueMM = 0;
        const int enumValueDots = 1;
        const int enumValueRevolutions = 2;

        public ObservableCollection<RxEnum<int>> UnitListColorOffset { get; set; }
        public ObservableCollection<RxEnum<int>> UnitListAngle { get; set; }
        public ObservableCollection<RxEnum<int>> UnitListRegister { get; set; }

        private RxEnum<int> _unitColorOffset;
        public RxEnum<int> UnitColorOffset
        {
            get { return _unitColorOffset; }
            set { _unitColorOffset = value; OnPropertyChanged("UnitColorOffset"); }
        }

        private RxEnum<int> _unitAngle;
        public RxEnum<int> UnitAngle
        {
            get { return _unitAngle; }
            set { _unitAngle = value; OnPropertyChanged("UnitAngle"); }
        }

        private RxEnum<int> _unitRegister { get; set; }
        public RxEnum<int> UnitRegister
        {
            get { return _unitRegister; }
            set { _unitRegister = value; OnPropertyChanged("UnitRegister"); }
        }

        public AlignmentSettings()
        {
            InitializeComponent();
            DataContext = this;

            var unitMM = new RxEnum<int>(enumValueMM, "mm");
            var unitDots = new RxEnum<int>(enumValueDots, "dots");
            var unitRevolutions = new RxEnum<int>(enumValueRevolutions, "Revolutions");

            UnitListColorOffset = new ObservableCollection<RxEnum<int>> { unitMM, unitDots };
            UnitListAngle = new ObservableCollection<RxEnum<int>> { unitRevolutions, unitMM };
            UnitListRegister = new ObservableCollection<RxEnum<int>> { unitMM, unitDots };

            if (RxGlobals.Alignment.AngleStitchCorrectionsUnitInRevolutions)
            {
                UnitAngle = unitRevolutions;
            }
            else
            {
                UnitAngle = unitMM;
            }
            if (RxGlobals.Alignment.ColorOffsetCorrectionUnitInDots)
            {
                UnitColorOffset = unitDots;
            }
            else
            {
                UnitColorOffset = unitMM;
            }
            if (RxGlobals.Alignment.RegisterCorrectionUnitInDots)
            {
                UnitRegister = unitDots;
            }
            else
            {
                UnitRegister = unitMM;
            }
        }

        private void Okay_Clicked(object sender, RoutedEventArgs e)
        {
            if (UnitAngle.Value == enumValueRevolutions)
            {
                RxGlobals.Alignment.AngleStitchCorrectionsUnitInRevolutions = true;
            }
            else if (UnitAngle.Value == enumValueMM)
            {
                RxGlobals.Alignment.AngleStitchCorrectionsUnitInRevolutions = false;
            }

            if (UnitColorOffset.Value == enumValueDots)
            {
                RxGlobals.Alignment.ColorOffsetCorrectionUnitInDots = true;
            }
            else if (UnitColorOffset.Value == enumValueMM)
            {
                RxGlobals.Alignment.ColorOffsetCorrectionUnitInDots = false;
            }

            if (UnitRegister.Value == enumValueDots)
            {
                RxGlobals.Alignment.RegisterCorrectionUnitInDots = true;
            }
            else if (UnitRegister.Value == enumValueMM)
            {
                RxGlobals.Alignment.RegisterCorrectionUnitInDots = false;
            }

            DialogResult = true;
        }

        private void Cancel_Clicked(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }
    }
}
