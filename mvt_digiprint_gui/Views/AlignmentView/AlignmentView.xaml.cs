using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.AlignmentView;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace RX_DigiPrint.Views.Alignment
{
    public class ColorDescription
    {
        public string ColorName { get; set; }
        public Brush Color { get; set; }
        public Brush StrokeColor { get; set; }
    }

    /// <summary>
    /// Interaction logic for AlignmentView.xaml
    /// </summary>
    public partial class AlignmentView : UserControl
    {
        private Models.Alignment _Alignment = RxGlobals.Alignment;

        private StackPanel InkCylinderSelectionPanel { get; set; }

        private int CurrentSelectedInkCylinderIndex { get; set; } // ink cylinder index (inside one color)
        private int CurrentSelectedColorIndex { get; set; }


        private List<ClusterAlignmentView> ClusterAlignmentViewList { get; set; }
        public AlignmentView()
        {
            InitializeComponent();
            DataContext = _Alignment;

            ClusterAlignmentViewList = new List<ClusterAlignmentView>();

            if (RxScreen.Screen.Surface)
            {
                InkCylinderSelectionPanel_Standard.Visibility = Visibility.Collapsed;
                InkCylinderSelectionPanel_Surface.Visibility = Visibility.Visible;
                InkCylinderSelectionPanel = InkCylinderSelectionPanel_Surface;
            }
            else
            {
                InkCylinderSelectionPanel_Standard.Visibility = Visibility.Visible;
                InkCylinderSelectionPanel_Surface.Visibility = Visibility.Collapsed;
                InkCylinderSelectionPanel = InkCylinderSelectionPanel_Standard;
            }

            _InitInkSupplySelection();
            RxGlobals.PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;

            CurrentSelectedColorIndex = -1;
            CurrentSelectedInkCylinderIndex = -1;

           
        }

        //--- PrintSystem_PropertyChanged ----------------------------------
        private void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("PrinterType") || e.PropertyName.Equals("ColorCnt") || e.PropertyName.Equals("HeadsPerColor")
                || e.PropertyName.Equals("InkCylindersPerColor"))
            {
                if (RxGlobals.PrintSystem.ColorCnt == 0 || RxGlobals.PrintSystem.InkCylindersPerColor == 0 ||
                    RxGlobals.PrintSystem.HeadsPerColor == 0)
                {
                    // no colors or print heads defined yet
                    return;
                }
                _InitInkSupplySelection();
            }
        }

        private void _InitInkSupplySelection()
        {
            int inkCylinderCnt = RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.InkCylindersPerColor;

            CurrentSelectedColorIndex = 0;
            CurrentSelectedInkCylinderIndex = 0;

            if (RxGlobals.PrintSystem.IsScanning)
            {
                CurrentSelectedColorIndex = RxGlobals.PrintSystem.ColorCnt - 1;
                CurrentSelectedInkCylinderIndex = RxGlobals.PrintSystem.InkCylindersPerColor - 1;
            }

            ColorSelectionPanel.Children.RemoveRange(0, ColorSelectionPanel.Children.Count);
            InkCylinderSelectionPanel.Children.Clear();

            Style colorSelectionStyle = null;
            try
            {
                colorSelectionStyle = Application.Current.FindResource("Mouvent.Alignment.ColorSelectionButtonStyle") as Style;
            }
            catch (Exception)
            {
                MvtMessageBox.Information("", "Could not load button style.");
                System.Environment.Exit(-1);
            }

            ColorDescription description = new ColorDescription();

            for (int color = 0; color < RxGlobals.PrintSystem.ColorCnt; color++)
            {
                int colorIndex = color;
                if (RxGlobals.PrintSystem.IsScanning) colorIndex = RxGlobals.PrintSystem.ColorCnt - 1 - color;

                UpdateColorDescription(colorIndex, ref description);
                RadioButton button = new RadioButton();
                button.Content = description.ColorName;
                button.Background = description.Color;
                button.BorderBrush = description.StrokeColor;
                button.Tag = colorIndex;
                button.Click += _ColorSelection_Clicked;
                button.Style = colorSelectionStyle;
                if (color == 0) button.IsChecked = true;
                else button.IsChecked = false;
                ColorSelectionPanel.Children.Add(button);
            }

            if (RxGlobals.PrintSystem.InkCylindersPerColor > 1)
            {
                InkCylinderSelectionPanel.Visibility = System.Windows.Visibility.Visible;
                Style cylinderSelectionStyle = null;
                try
                {
                    cylinderSelectionStyle = Application.Current.FindResource("Mouvent.Alignment.InkCylinderSelectionButtonStyle") as Style;
                }
                catch (Exception)
                {
                    MvtMessageBox.Information("", "Could not load button style.");
                    System.Environment.Exit(-1);
                }

                for (int cylinder = 0; cylinder < RxGlobals.PrintSystem.InkCylindersPerColor; cylinder++)
                {
                    int cylinderIndex = cylinder;
                    if (RxGlobals.PrintSystem.IsScanning) cylinderIndex = RxGlobals.PrintSystem.InkCylindersPerColor - 1 - cylinder;
                    RadioButton button = new RadioButton();
                    string displayIndex = (cylinderIndex + 1).ToString();
                    button.Content = "Cylinder " + displayIndex;
                    button.Tag = cylinderIndex;
                    button.Click += _InkCylinderSelection_Clicked;
                    button.Style = cylinderSelectionStyle;
                    if (cylinder == 0) button.IsChecked = true;
                    else button.IsChecked = false;
                    InkCylinderSelectionPanel.Children.Add(button);
                }
            }
            else
            {
                InkCylinderSelectionPanel.Visibility = System.Windows.Visibility.Collapsed;
            }

            if (inkCylinderCnt > 0)
            {
                int globalInkCylinderIndex = CurrentSelectedColorIndex * RxGlobals.PrintSystem.InkCylindersPerColor + CurrentSelectedInkCylinderIndex;
                _DrawClusters(globalInkCylinderIndex);
            }
        }

        private void _ColorSelection_Clicked(object sender, RoutedEventArgs e)
        {
            RadioButton button = sender as RadioButton;
            CurrentSelectedColorIndex = System.Convert.ToInt32(button.Tag);
            CurrentSelectedInkCylinderIndex = 0;
            
            // select first available ink cylinder:
            if (InkCylinderSelectionPanel.Children.Count > 0)
            {
                RadioButton inkCylinderButton = InkCylinderSelectionPanel.Children[0] as RadioButton;
                inkCylinderButton.IsChecked = true;
            }

            int globalInkCylinderIndex = CurrentSelectedColorIndex * RxGlobals.PrintSystem.InkCylindersPerColor + CurrentSelectedInkCylinderIndex;
            _DrawClusters(globalInkCylinderIndex);
        }

        private void _InkCylinderSelection_Clicked(object sender, RoutedEventArgs e)
        {
            RadioButton button = sender as RadioButton;
            CurrentSelectedInkCylinderIndex = System.Convert.ToInt32(button.Tag);

            int globalInkCylinderIndex = CurrentSelectedColorIndex * RxGlobals.PrintSystem.InkCylindersPerColor + CurrentSelectedInkCylinderIndex;
            _DrawClusters(globalInkCylinderIndex);
        }

        private void UpdateColorDescription(int colorIndex, ref ColorDescription description)
        {
            description.Color = Brushes.Transparent;
            description.StrokeColor = Brushes.Transparent;
            description.ColorName = "Color " + System.Convert.ToString(colorIndex + 1);

            int globalInkSupplyNo = colorIndex * RxGlobals.PrintSystem.InkCylindersPerColor; // first cylinder
            InkSupply inkSupply = null;
            string colorName = "";
            SolidColorBrush inkColor = null;
            if (globalInkSupplyNo < RxGlobals.InkSupply.List.Count)
            {
                inkSupply = RxGlobals.InkSupply.List[globalInkSupplyNo];
                if (inkSupply.InkType != null)
                {
                    colorName = inkSupply.InkType.Name;
                    inkColor = new SolidColorBrush(inkSupply.InkType.Color);
                }
                else
                {
                    return;
                }
            }
            else
            {
                // no color defined (yet) for this ink supply!
                return;
            }

            bool equal = true;
            for (int i = 1; i < RxGlobals.PrintSystem.InkCylindersPerColor; i++)
            {
                globalInkSupplyNo = i + colorIndex * RxGlobals.PrintSystem.InkCylindersPerColor;
                if (globalInkSupplyNo < RxGlobals.InkSupply.List.Count)
                {
                    inkSupply = RxGlobals.InkSupply.List[globalInkSupplyNo];
                    if (inkSupply.InkType != null)
                    {
                        if (!inkSupply.InkType.Name.Equals(colorName))
                        {
                            equal = false;
                            break;
                        }
                    }
                    else
                    {
                        equal = false;
                        break;
                    }
                }
                else
                {
                    // ink supply not listed!
                    equal = false;
                    break;
                }
            }

            if (equal)
            {
                description.ColorName = colorName;
                description.Color = inkColor;
                if (inkColor.Color.R == 255 && inkColor.Color.G == 255 && inkColor.Color.B == 255) description.StrokeColor = Brushes.Black;
            }
        }

        private void _DrawClusters(int inkSupplyNumber)
        {
            int inkCylinderCnt = RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.InkCylindersPerColor;
            int totalNumberOfClusters = ((inkCylinderCnt * RxGlobals.PrintSystem.HeadsPerInkCylinder) + 3) / 4;
            int firstClusterNumber = 0;
            int lastClusterNumber = 0;
            if (RxGlobals.PrintSystem.IsScanning)
            {
                int remainder = (4 - ((inkCylinderCnt * RxGlobals.PrintSystem.HeadsPerColor) % 4)) % 4;
                firstClusterNumber = (inkSupplyNumber * RxGlobals.PrintSystem.HeadsPerInkCylinder + remainder) / 4;
                lastClusterNumber = ((inkSupplyNumber * RxGlobals.PrintSystem.HeadsPerInkCylinder) + (RxGlobals.PrintSystem.HeadsPerInkCylinder - 1) + remainder) / 4;
            }
            else
            {
                firstClusterNumber = (inkSupplyNumber * RxGlobals.PrintSystem.HeadsPerInkCylinder) / 4;
                lastClusterNumber  = ((inkSupplyNumber * RxGlobals.PrintSystem.HeadsPerInkCylinder) + RxGlobals.PrintSystem.HeadsPerInkCylinder - 1) / 4;
            }

            // add views if neccessary:
            int clusterCount = totalNumberOfClusters;
            if (totalNumberOfClusters > 2) clusterCount = lastClusterNumber - firstClusterNumber + 1;
            for (int i = ClusterStackPanel.Children.Count; i < clusterCount; i++)
            {
                var view = new ClusterAlignmentView();
                ClusterAlignmentViewList.Add(view);
                ClusterStackPanel.Children.Add(view);
            }
            for (int i = 0; i < ClusterAlignmentViewList.Count; i++)
            {
                ClusterAlignmentViewList[i].Visibility = Visibility.Collapsed;
            }

            if (!RxGlobals.PrintSystem.IsScanning)
            {
                if (totalNumberOfClusters <= 2) // draw all clusters
                {
                    for (int i = 0; i < totalNumberOfClusters; i++)
                    {
                        ClusterAlignmentViewList[i].SetContext(i, i == 0 ? true : false, _Alignment.RobotIsConnected);
                        ClusterAlignmentViewList[i].Visibility = Visibility.Visible;
                    }

                    // color selection is not necessary
                    InkSupplySelectionPanel.Visibility = Visibility.Collapsed;
                }
                else
                {
                    for (int i = 0; i < clusterCount; i++)
                    {
                        int clusterIndex = firstClusterNumber + i;
                        ClusterAlignmentViewList[i].SetContext(clusterIndex, clusterIndex == firstClusterNumber ? true : false, _Alignment.RobotIsConnected);
                        ClusterAlignmentViewList[i].Visibility = Visibility.Visible;
                    }

                    InkSupplySelectionPanel.Visibility = Visibility.Visible;
                }
            }
            else
            {
                if (totalNumberOfClusters <= 2) // draw all clusters
                {
                    //for (int i = (totalNumberOfClusters - 1); i >= 0; i--)
                    for (int i = 0; i  < clusterCount; i++)
                    {
                        int index = totalNumberOfClusters - i - 1;
                        ClusterAlignmentViewList[i].SetContext(index, i == 0 ? true : false, _Alignment.RobotIsConnected);
                        ClusterAlignmentViewList[i].Visibility = Visibility.Visible;
                    }

                    // color selection is not necessary
                    InkSupplySelectionPanel.Visibility = Visibility.Collapsed;
                }
                else
                {
                    for (int i = 0; i < clusterCount; i++)
                    {
                        int clusterIndex = lastClusterNumber - i;
                        ClusterAlignmentViewList[i].SetContext(clusterIndex, i == 0 ? true : false, _Alignment.RobotIsConnected);
                        ClusterAlignmentViewList[i].Visibility = Visibility.Visible;
                    }
                    InkSupplySelectionPanel.Visibility = Visibility.Visible;
                }
            }
        }

        private void _DrawCluster(int clusterNumber, bool isFirstInView, bool robotIsConnected)
        {

            /*
                 ClusterAlignmentViewList[clusterNumber].SetContext(clusterNumber, isFirstInView, robotIsConnected);
                 ClusterAlignmentViewList[i].Visibility = Visibility.Visible;


             for (int i = clusterCount; i < ClusterStackPanel.Children.Count; i++)
             {
                 if (i < ClusterAlignmentViewList.Count)
                 {
                     ClusterAlignmentViewList[i].Visibility = Visibility.Collapsed;
                 }
             }
             */
        }

        private void CameraInputButton_Click(object sender, RoutedEventArgs e)
        {

        }

        private void ScanCheckImportButton_Click(object sender, RoutedEventArgs e)
        {
            _Alignment.ImportScanCheckValues();
        }

        private void ScanCheckClearButton_Click(object sender, RoutedEventArgs e)
        {
            bool result = MvtMessageBox.YesNo("Reset Values", "Do you want to reset all values?", MessageBoxImage.Question, false);
            if (result == true)
            {
                _Alignment.ClearCorrectionValues();
            }
        }

        private void SaveCorrectionValuesButton_Click(object sender, RoutedEventArgs e)
        {
            foreach (var elem in _Alignment.ClusterAlignmentDictionary)
            {
                elem.Value.SaveCorrectionValues();
            }

            RxGlobals.PrintSystem.SendMsg(TcpIp.CMD_SET_PRINTER_CFG);
            _Alignment.CorrectionValuesChanged = false;

            int globalInkCylinderIndex = CurrentSelectedColorIndex * RxGlobals.PrintSystem.InkCylindersPerColor + CurrentSelectedInkCylinderIndex;
            _DrawClusters(globalInkCylinderIndex);
        }

        private void RobotConnectButton_Click(object sender, RoutedEventArgs e)
        {
            bool isChecked = (bool)(sender as CheckBox).IsChecked;
            _Alignment.RobotIsConnected = isChecked;

            foreach (var elem in ClusterStackPanel.Children)
            {
                var view = elem as ClusterAlignmentView;
                view.ConnectRobot(isChecked);
            }
        }

        private void RobotDownloadButton_Click(object sender, RoutedEventArgs e)
        {
            MvtMessageBox.Information("Robot", "Robot functionality not implemented yet.");
        }

        private void SettingsButton_Click(object sender, RoutedEventArgs e)
        {
            AlignmentSettings settings = new AlignmentSettings();
            bool? result = settings.ShowDialog();
        }

        private void UserControl_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue)
            {
                _InitInkSupplySelection();
            }
        }
    }
}
