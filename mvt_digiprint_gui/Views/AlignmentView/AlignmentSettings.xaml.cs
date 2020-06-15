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

        const int enumValueUm = 0;
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

            var unitUm = new RxEnum<int>(enumValueUm, "µm");
            var unitDots = new RxEnum<int>(enumValueDots, "dots");
            var unitRevolutions = new RxEnum<int>(enumValueRevolutions, "Revolutions");

            UnitListColorOffset = new ObservableCollection<RxEnum<int>> { unitUm, unitDots };
            UnitListAngle = new ObservableCollection<RxEnum<int>> { unitRevolutions, unitUm };
            UnitListRegister = new ObservableCollection<RxEnum<int>> { unitUm, unitDots };

            if (RxGlobals.Alignment.AngleStitchCorrectionsUnitIsInUM)
            {
                UnitAngle = unitUm;
            }
            else
            {
                UnitAngle = unitRevolutions;
            }
            if (RxGlobals.Alignment.ColorOffsetCorrectionUnitInDots)
            {
                UnitColorOffset = unitDots;
            }
            else
            {
                UnitColorOffset = unitUm;
            }
            if (RxGlobals.Alignment.RegisterCorrectionUnitInDots)
            {
                UnitRegister = unitDots;
            }
            else
            {
                UnitRegister = unitUm;
            }
        }

        private void Okay_Clicked(object sender, RoutedEventArgs e)
        {
            if (UnitAngle.Value == enumValueUm)
            {
                RxGlobals.Alignment.AngleStitchCorrectionsUnitIsInUM = true;
            }
            else if (UnitAngle.Value == enumValueRevolutions)
            {
                RxGlobals.Alignment.AngleStitchCorrectionsUnitIsInUM = false;
            }

            if (UnitColorOffset.Value == enumValueDots)
            {
                RxGlobals.Alignment.ColorOffsetCorrectionUnitInDots = true;
            }
            else if (UnitColorOffset.Value == enumValueUm)
            {
                RxGlobals.Alignment.ColorOffsetCorrectionUnitInDots = false;
            }

            if (UnitRegister.Value == enumValueDots)
            {
                RxGlobals.Alignment.RegisterCorrectionUnitInDots = true;
            }
            else if (UnitRegister.Value == enumValueUm)
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
