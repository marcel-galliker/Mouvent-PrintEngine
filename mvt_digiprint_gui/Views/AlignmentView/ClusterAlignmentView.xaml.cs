using MahApps.Metro.IconPacks;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using RX_DigiPrint.Models;
using RX_Common;
using RX_DigiPrint.Views.Alignment;
using RX_DigiPrint.Helpers;

namespace RX_DigiPrint.Views.Alignment
{
    /// <summary>
    /// Interaction logic for ClusterAlignmentView.xaml
    /// </summary>
    public partial class ClusterAlignmentView : UserControl, INotifyPropertyChanged
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

        private bool _robotIsConnected;
        public bool RobotIsConnected
        {
            get { return _robotIsConnected; }
            set { _robotIsConnected = value; OnPropertyChanged("RobotIsConnected"); }
        }

        private float LastAngleValue { get; set; }
        
        private const int kHeadsPerCluster = 4;

        private RX_DigiPrint.Models.Alignment _Alignment = RxGlobals.Alignment;

        private int ClusterNumber { get; set; }

        public ClusterAlignmentView()
        {
            InitializeComponent();
        }

        public bool SetContext(int clusterNumber, bool isFirstInView, bool robotIsConnected)
        { 
            ClusterNumber = clusterNumber;
            
            if (clusterNumber < _Alignment.ClusterAlignmentDictionary.Count)
            {
                _Alignment.ClusterAlignmentDictionary[clusterNumber].SetIsFirstInView(isFirstInView);
                _Alignment.ClusterAlignmentDictionary[clusterNumber].SetIsScanning(RxGlobals.PrintSystem.IsScanning);
                _Alignment.ClusterAlignmentDictionary[clusterNumber].SetRobotIsConnected(robotIsConnected);
                DataContext = _Alignment.ClusterAlignmentDictionary[clusterNumber];
            }
            else
            {
                return false;
            }

            RobotIsConnected = RxGlobals.Alignment.RobotIsConnected; // todo: get value

            _InitPrintHeadColors();

            return true;
        }

        private void ClusterAlignmentViewDataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {

        }

        private void _InitPrintHeadColors()
        {
            List<Button> printHeadColorButtons = new List<Button>{ PrintHeadColor0, PrintHeadColor1, PrintHeadColor2, PrintHeadColor3 };
            SolidColorBrush brush = Brushes.WhiteSmoke;
            for (int h = 0; h < kHeadsPerCluster; h++)
            {
                int globalHeadNumber = 0;
                if (RxGlobals.PrintSystem.IsScanning)
                {
                    int remainder = (4 - ((RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.HeadsPerColor) % 4)) % 4;
                    globalHeadNumber = ClusterNumber * kHeadsPerCluster + h - remainder;
                }
                else
                {
                    globalHeadNumber = ClusterNumber * kHeadsPerCluster + h;
                }
                Button button = printHeadColorButtons[h];
                if (RxGlobals.PrintSystem.IsScanning) button = printHeadColorButtons[3 - h];
                if (globalHeadNumber >= 0)
                {
                    brush = Brushes.WhiteSmoke;
                    if (RxGlobals.PrintSystem.HeadsPerColor != 0)
                    {
                        int inkCylinderNumber = globalHeadNumber / RxGlobals.PrintSystem.HeadsPerInkCylinder;
                        if (RxGlobals.InkSupply.List[inkCylinderNumber].InkType != null)
                        {
                            brush = new SolidColorBrush(RxGlobals.InkSupply.List[inkCylinderNumber].InkType.Color);
                        }
                    }
                }
                else
                {
                    brush = Brushes.WhiteSmoke;
                }
                button.BorderBrush = brush;

                // Stroke:
                button.Background = Brushes.Transparent;
                if (brush.Color == Brushes.White.Color || brush.Color == Brushes.WhiteSmoke.Color)
                {
                    button.Background = Brushes.Gray;
                }
            }
        }

        private void AngleInfoButton_Click(object sender, RoutedEventArgs e)
        {
            double correctionValue = (float)0.0;
            int tag = -1;
            ClusterAlignment clusterAlignment = null;
            
            try
            {
                tag = Convert.ToInt32((sender as MvtButton).Tag);
                clusterAlignment = DataContext as ClusterAlignment;
            }
            catch
            {
                // software internal problem!
                MvtMessageBox.InformationAndExit("Error", "");
            }

            if (tag == -1 || clusterAlignment == null)
            {
                return;
            }
            
            if (RxGlobals.Alignment.ScanCheckValuesAvailable == true)
            {
                try
                {
                    correctionValue = clusterAlignment.AngleCorrectionValues[tag].Correction;
                }
                catch
                {
                    // software internal problem!
                    MvtMessageBox.InformationAndExit("Error", "");
                }
            }

            HeadAdjustmentView.HeadAdjustmentType type = HeadAdjustmentView.HeadAdjustmentType.TypeAngle;
            HeadAdjustmentView wnd = new HeadAdjustmentView(type, RxGlobals.Alignment.ScanCheckValuesAvailable, correctionValue, RobotIsConnected);
            bool result = (bool)wnd.ShowDialog();
            if (result == true)
            {
                clusterAlignment.AngleCorrectionValues[tag] = new AngleCorrectionValue { Correction = wnd.CorrectionValue };
                if (correctionValue != wnd.CorrectionValue)
                {
                    RxGlobals.Alignment.CorrectionValuesChanged = true;
                }
            }
            
        }

        private void StitchInfoButton_Click(object sender, RoutedEventArgs e)
        {
            double correctionValue = (float)0.0;
            int tag = -1;
            ClusterAlignment clusterAlignment = null;
            try
            {
                tag = Convert.ToInt32((sender as MvtButton).Tag);
                clusterAlignment = DataContext as ClusterAlignment;
            }
            catch 
            {
                // software internal problem!
                MvtMessageBox.InformationAndExit("Error", "");
            }

            if (tag == -1 || clusterAlignment == null)
            {
                return;
            }

           
            if (RxGlobals.Alignment.ScanCheckValuesAvailable == true)
            {
                try
                {
                    correctionValue = clusterAlignment.StitchCorrectionValues[tag].Correction;
                }
                catch (Exception)
                {
                    // software internal problem!
                    MvtMessageBox.InformationAndExit("Error", "");
                }
            }
            
            HeadAdjustmentView.HeadAdjustmentType type = HeadAdjustmentView.HeadAdjustmentType.TypeStitch;
            HeadAdjustmentView wnd = new HeadAdjustmentView(type, RxGlobals.Alignment.ScanCheckValuesAvailable, correctionValue, RobotIsConnected);
            bool result = (bool)wnd.ShowDialog();
            if (result == true)
            {              
                clusterAlignment.StitchCorrectionValues[tag] = new StitchCorrectionValue { Correction = wnd.CorrectionValue };
                if (correctionValue != wnd.CorrectionValue)
                {
                    RxGlobals.Alignment.CorrectionValuesChanged = true;
                }
            }
            
        }

        private void RegisterCorrectionButton_Click(object sender, RoutedEventArgs e)
        {
            double correctionValue = (float)0.0;
            int tag = -1;
            ClusterAlignment clusterAlignment = null;
            try
            {
                tag = Convert.ToInt32((sender as MvtButton).Tag);
                clusterAlignment = DataContext as ClusterAlignment;
            }
            catch
            {
                // software internal problem!
                MvtMessageBox.InformationAndExit("Error", "");
            }

            if (tag == -1 || clusterAlignment == null)
            {
                return;
            }


            if (RxGlobals.Alignment.ScanCheckValuesAvailable == true)
            {
                try
                {
                    correctionValue = clusterAlignment.RegisterCorrectionValues[tag].Correction;
                }
                catch (Exception)
                {
                    // software internal problem!
                    MvtMessageBox.InformationAndExit("Error", "");
                }
            }

            SoftwareValueCorrectionView.SoftwareValueType type = SoftwareValueCorrectionView.SoftwareValueType.TypeRegister;
            SoftwareValueCorrectionView wnd = new SoftwareValueCorrectionView(type, RxGlobals.Alignment.ScanCheckValuesAvailable, correctionValue);
            bool result = (bool)wnd.ShowDialog();
            if (result == true)
            {
                clusterAlignment.RegisterCorrectionValues[tag] = new DotsCorrectionValue { Correction = wnd.CorrectionValue };
                if(correctionValue != wnd.CorrectionValue)
                {
                    RxGlobals.Alignment.CorrectionValuesChanged = true;
                }
            }
        }

        private void BackwardRegisterCorrectionButton_Click(object sender, RoutedEventArgs e)
        {
            double correctionValue = (float)0.0;
            int tag = -1;
            ClusterAlignment clusterAlignment = null;
            try
            {
                tag = Convert.ToInt32((sender as MvtButton).Tag);
                clusterAlignment = DataContext as ClusterAlignment;
            }
            catch
            {
                // software internal problem!
                MvtMessageBox.InformationAndExit("Error", "");
            }

            if (tag == -1 || clusterAlignment == null)
            {
                return;
            }


            if (RxGlobals.Alignment.ScanCheckValuesAvailable == true)
            {
                try
                {
                    correctionValue = clusterAlignment.BackwardRegisterCorrectionValues[tag].Correction;
                }
                catch (Exception)
                {
                    // software internal problem!
                    MvtMessageBox.InformationAndExit("Error", "");
                }
            }

            SoftwareValueCorrectionView.SoftwareValueType type =  SoftwareValueCorrectionView.SoftwareValueType.TypeRegisterBackwards;
            SoftwareValueCorrectionView wnd = new SoftwareValueCorrectionView(type, RxGlobals.Alignment.ScanCheckValuesAvailable, correctionValue);
            bool result = (bool)wnd.ShowDialog();
            if (result == true)
            {
                clusterAlignment.BackwardRegisterCorrectionValues[tag] = new DotsCorrectionValue { Correction = wnd.CorrectionValue };
                if (correctionValue != wnd.CorrectionValue)
                {
                    RxGlobals.Alignment.CorrectionValuesChanged = true;
                }
            }
        }

        private void ColorOffsetInfoButton_Click(object sender, RoutedEventArgs e)
        {
            double correctionValue = (float)0.0;
            int tag = -1;
            ClusterAlignment clusterAlignment = null;
            try
            {
                tag = Convert.ToInt32((sender as MvtButton).Tag);
                clusterAlignment = DataContext as ClusterAlignment;
            }
            catch
            {
                // software internal problem!
                MvtMessageBox.InformationAndExit("Error", "");
            }

            if (tag == -1 || clusterAlignment == null)
            {
                return;
            }

            if (RxGlobals.Alignment.ScanCheckValuesAvailable == true)
            {
                try
                {
                    correctionValue = clusterAlignment.ColorOffsetCorrectionValues[tag].Correction;
                }
                catch (Exception)
                {
                    // software internal problem!
                    MvtMessageBox.InformationAndExit("Error", "");
                }
            }

            SoftwareValueCorrectionView.SoftwareValueType type = SoftwareValueCorrectionView.SoftwareValueType.TypeColorOffset;
            SoftwareValueCorrectionView wnd = new SoftwareValueCorrectionView(type, RxGlobals.Alignment.ScanCheckValuesAvailable, correctionValue);
            bool result = (bool)wnd.ShowDialog();
            if (result == true)
            {
                clusterAlignment.ColorOffsetCorrectionValues[tag] = new DotsCorrectionValue { Correction = wnd.CorrectionValue };
                if (correctionValue != wnd.CorrectionValue)
                {
                    RxGlobals.Alignment.CorrectionValuesChanged = true;
                }
            }
        }

        private void RobotDownloadButton_Click(object sender, RoutedEventArgs e)
        {
            // todo
        }

        public void ConnectRobot(bool connect)
        {
            RobotIsConnected = connect;
            ClusterAlignment clusterAlignment = DataContext as ClusterAlignment;
            clusterAlignment.SetRobotIsConnected(connect);
        }

        private void CorrectionValueNumBoxLostFocus(object sender, RoutedEventArgs e)
        {
            try
            {
                // Check whether values have been changed
                RX_Common.MvtNumBox box = sender as RX_Common.MvtNumBox;
                if (box.TextBoxContentChanged == true)
                {
                    RxGlobals.Alignment.CorrectionValuesChanged = true;
                }
            }
            catch
            {
                // software internal problem!
                MvtMessageBox.InformationAndExit("Error", "");
            }
        }

        private void RegisterCorrectionValueNumBoxLostFocus(object sender, RoutedEventArgs e)
        {
            RX_Common.MvtNumBox box = null;
            try
            {
                // Check whether values have been changed
                box = sender as RX_Common.MvtNumBox;
                if (box.TextBoxContentChanged == true)
                {
                    RxGlobals.Alignment.CorrectionValuesChanged = true;
                }
            }
            catch
            {
                // software internal problem!
                MvtMessageBox.InformationAndExit("Error", "");
            }

            if (RxGlobals.Alignment.CorrectionValuesChanged == true)
            {
                int h = 0;
                double difference = box.Difference;
                try
                {
                    h = Convert.ToInt32(box.Tag);
                }
                catch
                {
                    // software internal problem!
                    MvtMessageBox.InformationAndExit("Error", "");
                }

                int globalHeadNumber = -1;
                if (RxGlobals.PrintSystem.IsScanning)
                {
                    int remainder = (4 - ((RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.HeadsPerColor) % 4)) % 4;
                    globalHeadNumber = ClusterNumber * kHeadsPerCluster + h - remainder;
                }
                else
                {
                    globalHeadNumber = ClusterNumber * kHeadsPerCluster + h;
                }

                if (globalHeadNumber >= 0)
                {
                       
                    RxGlobals.Alignment.UpdateRegisterCorrectionValues(ClusterNumber, globalHeadNumber, difference);
                }
            }

        }


        public void SaveCorrectionValues()
        {
            ClusterAlignment clusterAlignment = DataContext as ClusterAlignment;
            clusterAlignment.SaveCorrectionValues();
        }

        /*
        private void ColorOffsetCorrectionUnitToggle_Clicked(object sender, RoutedEventArgs e)
        {
            
            if (ColorOffsetCorrectionToggleSwitch.IsChecked == true)
            {
                RxGlobals.Alignment.ColorOffsetCorrectionUnitInDots = true;
            }
            else
            {
                RxGlobals.Alignment.ColorOffsetCorrectionUnitInDots = false;
            }
            
        }

        private void AngleCorrectionUnitToggle_Clicked(object sender, RoutedEventArgs e)
        {            
            
            if (AngleCorrectionToggleSwitch.IsChecked == true)
            {
                RxGlobals.Alignment.AngleStitchCorrectionsUnitIsInUM = true;
            }
            else
            {
                RxGlobals.Alignment.AngleStitchCorrectionsUnitIsInUM = false;
            }
            RxGlobals.Alignment.UpdateUnits();
        }

        private void RegisterCorrectionUnitToggle_Clicked(object sender, RoutedEventArgs e)
        {
            if (RegisterCorrectionToggleSwitch.IsChecked == true)
            {
                RxGlobals.Alignment.RegisterCorrectionUnitInDots = true;
            }
            else
            {
                RxGlobals.Alignment.RegisterCorrectionUnitInDots = false;
            }
            RxGlobals.Alignment.UpdateUnits();
        }
        */
    }
}
