using RX_Common;
using RX_DigiPrint.Converters;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Media;

namespace RX_DigiPrint.Views.PrintSystemExtendedView
{
    public enum ETotalStatus
    {
        STATUS_UNDEF,
        STATUS_OKAY,
        STATUS_WARNING,
        STATUS_ERROR,
    }

    public class InkCylinderStatus : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        private PrintSystem _PrintSystem = RxGlobals.PrintSystem;

        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }
        private ETotalStatus _status;
        public ETotalStatus Status
        {
            get { return _status; }
            set
            {
                _status = value;
                OnPropertyChanged("Status");
            }
        }
    }

    public class PrintHeadStatus : INotifyPropertyChanged
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
        private ETotalStatus _status;
        public ETotalStatus Status
        {
            get { return _status; }
            set
            {
                _status = value;
                OnPropertyChanged("Status");
                StatusColor = Brushes.Transparent;
                switch (value)
                {
                    case ETotalStatus.STATUS_OKAY:
                        StatusColor = Brushes.SeaGreen;
                        break;
                    case ETotalStatus.STATUS_WARNING:
                        StatusColor = Rx.BrushWarn;
                        break;
                    case ETotalStatus.STATUS_ERROR:
                        StatusColor = Rx.BrushError;
                        break;
                    default:
                        StatusColor = Brushes.Transparent;
                        break;
                }
            }
        }

        private Brush _StatusColor;
        public Brush StatusColor
        {
            get { return _StatusColor; }
            set
            {
                _StatusColor = value;
                OnPropertyChanged("StatusColor");
            }
        }
    }

    /// <summary>
    /// Interaction logic for PrintSystemExtendedView.xaml
    /// </summary>
    public partial class PrintSystemExtendedView : UserControl, INotifyPropertyChanged
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

        private ObservableCollectionEx<InkCylinderStatus> _InkCylinderStatusList;
        public ObservableCollectionEx<InkCylinderStatus> InkCylinderStatusList
        {
            get { return _InkCylinderStatusList; }
            set
            {
                _InkCylinderStatusList = value;
                OnPropertyChanged("InkCylinderStatusList");
            }
        }

        private bool _Init { get; set; }

        private ObservableCollectionEx<ObservableCollectionEx<ObservableCollection<PrintHeadStatus>>> _PrintHeadStatusList;
        public ObservableCollectionEx<ObservableCollectionEx<ObservableCollection<PrintHeadStatus>>> PrintHeadStatusList
        {
            get { return _PrintHeadStatusList; }
            set
            {
                _PrintHeadStatusList = value;
                OnPropertyChanged("PrintHeadStatusList");
            }
        }

        private List<MvtInkSupplyRadioButton> InkCylinderViewButtons { get; set; }
        private MvtInkSupplyRadioButton CurrentlyCheckedButton { get; set; }
        private List<ColorPanelView> ColorPanelViewList { get; set; }
        private int NumberOfGridRowsPerColor = 4;
        private PrintSystem _PrintSystem = RxGlobals.PrintSystem;

        private DetailsView DetailsView = null;

        private const double FLOW_RESISTANCE_MAX = 1.1;

        public PrintSystemExtendedView()
        {
            InitializeComponent();
            DataContext = _PrintSystem;

            InkCylinderViewButtons = new List<MvtInkSupplyRadioButton>();
            ColorPanelViewList = new List<ColorPanelView>();
            InkCylinderStatusList = new ObservableCollectionEx<InkCylinderStatus>();
            PrintHeadStatusList = new ObservableCollectionEx<ObservableCollectionEx<ObservableCollection<PrintHeadStatus>>>();
            Init();

            _PrintSystem.PropertyChanged += PrintSystem_PropertyChanged;
            RxGlobals.User.PropertyChanged += User_PropertyChanged;
            _Init = false;
            RxGlobals.Timer.TimerFct += _timer_Tick;
        }

        private void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("ColorCnt") || e.PropertyName.Equals("HeadsPerColor")
                || e.PropertyName.Equals("InkCylindersPerColor") 
                || e.PropertyName.Equals("PrinterType"))
            {
                if (_PrintSystem.ColorCnt == 0 || _PrintSystem.HeadsPerColor == 0 
                ||  _PrintSystem.InkCylindersPerColor == 0
                    )
                {
                    // no colors or heads defined
                    return;
                }

                InkCylinderViewButtons.Clear();
                CurrentlyCheckedButton = null;

             //   if (_PrintSystem.ColorCnt*_PrintSystem.HeadsPerColor < 16)
                if (_PrintSystem.PrinterType!=EPrinterType.printer_CB612)
                {
                    // it does not make sense to display print system like this!
                    // display "old" PrintSystemView
                    OverviewPanel.Children.Clear();
                    OverviewPanel.RowDefinitions.Clear();
                    OverviewPanel.ColumnDefinitions.Clear();
                    return;
                }

                Init();

                if (!(_PrintSystem.ColorCnt == 0 || _PrintSystem.HeadsPerColor == 0 || _PrintSystem.InkCylindersPerColor == 0))
                {
                    int headCnt = _PrintSystem.ColorCnt * _PrintSystem.HeadsPerColor;
                    
                    for (int i = 0; i < headCnt; i++)
                    {
                        TcpIp.SDisabledJetsMsg msg = new TcpIp.SDisabledJetsMsg();
                        msg.head = i;
                        RxGlobals.RxInterface.SendMsg(TcpIp.CMD_GET_DISABLED_JETS, ref msg);
                    }
                }
            }
        }

        public void InkCylinderInkTypeChanged()
        {
            foreach (var elem in ColorPanelViewList)
            {
                elem.UpdateColorDescription();
            }
        }

        private void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (DetailsView != null)
            {
                DetailsView.SetUser();
            }
        }

        private void Init()
        {
            if (_PrintSystem.ColorCnt == 0 || _PrintSystem.HeadsPerColor == 0)
            {
                return;
            }

            /*
            double dClustersPerInkCylinder = (double)(_PrintSystem.PrintHeadsPerInkCylinderCnt) / (double)TcpIp.HEAD_CNT;
            int clustersPerInkCylinder = _PrintSystem.PrintHeadsPerInkCylinderCnt / TcpIp.HEAD_CNT;
            if (dClustersPerInkCylinder - clustersPerInkCylinder != 0 || clustersPerInkCylinder < 1)
            {
                // there must be at least one cluster per color!
                return;
            }
            */

            OverviewPanel.Children.Clear();
            OverviewPanel.RowDefinitions.Clear();
            OverviewPanel.ColumnDefinitions.Clear();
            InkCylinderStatusList.Clear();
            PrintHeadStatusList.Clear();
            ColorPanelViewList.Clear();

            DetailsView = new DetailsView(this);

            for (int r = 0; r < _PrintSystem.ColorCnt; r++)
            {
                OverviewPanel.RowDefinitions.Add(new RowDefinition() { Height = GridLength.Auto }); // Color Header
                OverviewPanel.RowDefinitions.Add(new RowDefinition() { Height = GridLength.Auto }); // Ink Cylinder Headers
                OverviewPanel.RowDefinitions.Add(new RowDefinition() { Height = GridLength.Auto }); // Space
                OverviewPanel.RowDefinitions.Add(new RowDefinition() { Height = GridLength.Auto }); // Details View
            }
            for (int c = 0; c < _PrintSystem.InkCylindersPerColor; c++)
            {
                OverviewPanel.ColumnDefinitions.Add(new ColumnDefinition() { Width = new GridLength(1, GridUnitType.Star) });
            }

            for (int colorIndex = 0; colorIndex < _PrintSystem.ColorCnt; colorIndex++)
            {
                var colorPanelView = new ColorPanelView(colorIndex);
                if (colorIndex == 0) colorPanelView.Margin = new Thickness(0);
                else colorPanelView.Margin = new Thickness(0, 10, 0, 0);
                Grid.SetRow(colorPanelView, colorIndex * NumberOfGridRowsPerColor);
                Grid.SetColumn(colorPanelView, 0);
                Grid.SetColumnSpan(colorPanelView, _PrintSystem.InkCylindersPerColor);
                OverviewPanel.Children.Add(colorPanelView);

                int clustersPerColor = (_PrintSystem.HeadsPerColor+TcpIp.HEAD_CNT-1) / TcpIp.HEAD_CNT;
                int clustersPerInkCylinder = clustersPerColor / _PrintSystem.InkCylindersPerColor;

                for (int inkCylinderIndex = 0; inkCylinderIndex < _PrintSystem.InkCylindersPerColor; inkCylinderIndex++)
                {
                    MvtInkSupplyRadioButton button = new MvtInkSupplyRadioButton();
                    button.ToolTip = GetClusterNumbers(colorIndex, inkCylinderIndex);

                    try
                    {
                        button.Style = (Style)FindResource("InkCylinderButtonStyle");
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine(e.Message);
                    }
                    button.HorizontalContentAlignment = System.Windows.HorizontalAlignment.Stretch;

                    button.Click += InkCylinderClicked;
                    button.Tag = colorIndex * 100 + inkCylinderIndex;
                    if (inkCylinderIndex == 0) button.Margin = new Thickness(0, 5, 0, 0);
                    else button.Margin = new Thickness(10, 5, 0, 0);

                    StackPanel clusterPanel = new StackPanel();
                    clusterPanel.Orientation = Orientation.Horizontal;

                    for (int clusterIndex = 0; clusterIndex < clustersPerInkCylinder; clusterIndex++)
                    {
                        Border clusterBorder = new Border();
                        clusterBorder.HorizontalAlignment = System.Windows.HorizontalAlignment.Stretch;
                        clusterBorder.BorderThickness = new Thickness(0);
                        clusterBorder.BorderBrush = Brushes.WhiteSmoke;
                        clusterBorder.Margin = new Thickness(0, 5, 10, 5);
                        ItemsControl itemsControl = new ItemsControl();

                        try
                        {
                            itemsControl.ItemTemplate = (System.Windows.DataTemplate)FindResource("DataTemplate_Level1");
                        }
                        catch (Exception e)
                        {
                            Console.WriteLine(e.Message);
                        }

                        PrintHeadStatusList.Add(new ObservableCollectionEx<ObservableCollection<PrintHeadStatus>>()); // one per cluster
                        var clusterList = PrintHeadStatusList.Last();
                        clusterList.Add(new ObservableCollection<PrintHeadStatus>());
                        for (int i = 0; i < TcpIp.HEAD_CNT; i++)
                        {
                            clusterList[0].Add(new PrintHeadStatus { Status = ETotalStatus.STATUS_WARNING });
                        }

                        itemsControl.ItemsSource = clusterList;

                        clusterBorder.Child = itemsControl;
                        clusterPanel.Children.Add(clusterBorder);
                    }

                    button.Content = clusterPanel;
                    Grid.SetRow(button, colorIndex * NumberOfGridRowsPerColor + 1);
                    Grid.SetColumn(button, inkCylinderIndex);
                    OverviewPanel.Children.Add(button);
                    InkCylinderViewButtons.Add(button);

                    InkCylinderStatusList.Add(new InkCylinderStatus { Status = ETotalStatus.STATUS_UNDEF });
                    Binding inkCylinderStatusBinding = new Binding("Status");
                    inkCylinderStatusBinding.Source = InkCylinderStatusList[InkCylinderStatusList.Count - 1];
                    InkCylinderGlobalStatusConverter inkCylinderGlobalStatusConverter = new InkCylinderGlobalStatusConverter();
                    inkCylinderStatusBinding.Converter = inkCylinderGlobalStatusConverter;
                    button.SetBinding(MvtInkSupplyRadioButton.BackgroundProperty, inkCylinderStatusBinding);
                }

                colorPanelView.UpdateColorDescription();
                ColorPanelViewList.Add(colorPanelView);

                _Init = true;
            }

            for (int i = 0; i < _PrintSystem.InkCylindersPerColor * _PrintSystem.ColorCnt; i++)
            {
                UpdateInkCylinderStatus(i);
            }
            UpdatePrintHeadStatus();
        }

        private string GetClusterNumbers(int colorIndex, int inkCylinderIndex)
        {
        //  int firstHead = colorIndex*_PrintSystem.HeadsPerColor + inkCylinderIndex*_PrintSystem.HeadsPerInkCylinder;            
            int firstHead = inkCylinderIndex*_PrintSystem.HeadsPerInkCylinder;            
            int lastHead  = firstHead+_PrintSystem.HeadsPerInkCylinder-1;
            string colorName="?";
            InkType inktype= RxGlobals.InkSupply.List[colorIndex*_PrintSystem.InkCylindersPerColor].InkType;
            if (inktype!=null) colorName=InkType.ColorNameShort(inktype.ColorCode);
            return string.Format("Heads {0}-{1}...{0}-{2}", colorName, (firstHead+1).ToString(), (lastHead+1).ToString());

            /*
            int firstClusterNumber     = (firstHead+TcpIp.HEAD_CNT-1) / TcpIp.HEAD_CNT;
            int lastClusterNumber      = (firstHead+headsPerInkCylinder-1+TcpIp.HEAD_CNT-1) / TcpIp.HEAD_CNT;

            if (firstClusterNumber == lastClusterNumber)
            {
                return "Cluster " + (firstClusterNumber + 1).ToString();
            }
            else
            {
                return "Clusters " + (firstClusterNumber + 1).ToString() + "-" + (lastClusterNumber + 1).ToString();
            }
            */
        }

        private void UpdateInkCylinderStatus(int inkCylinderIndex)
        {
            if (!_Init)
                return;

            if (RxGlobals.InkSupply.List.Count == 0)
                return;
            if (!(inkCylinderIndex < RxGlobals.InkSupply.List.Count))
            {
                InkCylinderStatusList[inkCylinderIndex] = new InkCylinderStatus { Status = ETotalStatus.STATUS_UNDEF };
                return;
            }

            InkSupply inkSupply = null;
            inkSupply = RxGlobals.InkSupply.List[inkCylinderIndex];
            
            if (inkSupply == null || inkCylinderIndex >= InkCylinderStatusList.Count)
            {
                InkCylinderStatusList[inkCylinderIndex] = new InkCylinderStatus { Status = ETotalStatus.STATUS_UNDEF };
                return;
            }


            if (inkSupply.Err != 0 || !inkSupply.Connected)
            {
                InkCylinderStatusList[inkCylinderIndex].Status = ETotalStatus.STATUS_ERROR;
            }
            else if (!inkSupply.TempReady
                || !inkSupply.CondTempReady
                || inkSupply.CanisterErr >= RX_DigiPrint.Services.ELogType.eErrWarn
                || inkSupply.Warn)
            {
                InkCylinderStatusList[inkCylinderIndex].Status = ETotalStatus.STATUS_WARNING;
            }
            else
            {
                InkCylinderStatusList[inkCylinderIndex].Status = ETotalStatus.STATUS_OKAY;
            }
        }

        public IEnumerable<T> FindVisualChildren<T>(DependencyObject depObj) where T : DependencyObject
        {
            if (depObj != null)
            {
                for (int i = 0; i < VisualTreeHelper.GetChildrenCount(depObj); i++)
                {
                    DependencyObject child = VisualTreeHelper.GetChild(depObj, i);

                    if (child != null && child is T)
                        yield return (T)child;

                    foreach (T childOfChild in FindVisualChildren<T>(child))
                        yield return childOfChild;
                }
            }
        }

        private void UpdatePrintHeadStatus()
        {
            if (!_Init)
                return;

            for (int colorIndex = 0; colorIndex < _PrintSystem.ColorCnt; colorIndex++)
            {
                for (int inkCylinderIndex = 0; inkCylinderIndex < _PrintSystem.InkCylindersPerColor; inkCylinderIndex++)
                {
                    int HeadsPerInkCylinder = _PrintSystem.HeadsPerColor / _PrintSystem.InkCylindersPerColor;
                    int clustersPerInkCylinder = _PrintSystem.Cluster(_PrintSystem.HeadsPerInkCylinder);
                    for (int clusterIndex = 0; clusterIndex < clustersPerInkCylinder; clusterIndex++)
                    {
                        int globalClusterNumber = clusterIndex + inkCylinderIndex * clustersPerInkCylinder + colorIndex * (clustersPerInkCylinder * _PrintSystem.InkCylindersPerColor);

                        for (int headIndex = 0; headIndex < TcpIp.HEAD_CNT; headIndex++)
                        {
                            try
                            {
                                PrintHeadStatus status = new PrintHeadStatus { Status = ETotalStatus.STATUS_UNDEF };
                                int globalPrintHeadIndex = headIndex + globalClusterNumber * TcpIp.HEAD_CNT;
                                var printHeadStatistics = RxGlobals.HeadStat.List[globalPrintHeadIndex];

                                if (!printHeadStatistics.Connected || printHeadStatistics.Err != 0 || printHeadStatistics.FlowResistance >= FLOW_RESISTANCE_MAX)
                                {
                                    status.Status = ETotalStatus.STATUS_ERROR;
                                }
                                else if (printHeadStatistics.Warn)
                                {
                                    status.Status = ETotalStatus.STATUS_WARNING;
                                }
                                else
                                {
                                    status.Status = ETotalStatus.STATUS_OKAY;
                                }

                                PrintHeadStatusList[globalClusterNumber][0][headIndex] = status;
                            }
                            catch (Exception ex)
                            {
                                Console.WriteLine(ex.Message);
                            }
                        }
                    }
                }
            }
        }

        private void InkCylinderClicked(object sender, RoutedEventArgs e)
        {
            MvtInkSupplyRadioButton button = sender as MvtInkSupplyRadioButton;

            if (button == null)
            {
                return;
            }
            if (button.IsChecked == true && button.Equals(CurrentlyCheckedButton))
            {
                button.IsChecked = false;
                CurrentlyCheckedButton = null;
            }

            foreach (var btn in InkCylinderViewButtons)
            {
                if (!btn.Equals(button))
                {
                    btn.IsChecked = false;
                }
            }

            int inkCylinderIndex = (int)button.Tag % 100;
            if (inkCylinderIndex < 0)
            {
                // error handling!
                return;
            }

            int colorIndex = (int)button.Tag / 100;

            try
            {
                OverviewPanel.Children.Remove(DetailsView);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                return;
            }

            if (button.IsChecked == true)
            {
                CurrentlyCheckedButton = button;

                foreach (var btn in InkCylinderViewButtons)
                {
                    if (!btn.Equals(button))
                    {
                        btn.ShowStatusBorder = false;
                    }
                }

                int globalInkCylinderIndex = inkCylinderIndex + colorIndex * _PrintSystem.InkCylindersPerColor;
                DrawDetailsView(colorIndex, globalInkCylinderIndex);
            }
            else
            {
                foreach (var btn in InkCylinderViewButtons)
                {
                    btn.ShowStatusBorder = true;
                }
            }

            DrawGridLines(colorIndex, inkCylinderIndex, (bool)button.IsChecked);
        }

        private void DrawDetailsView(int colorIndex, int inkCylinderIndex)
        {
            DetailsView.SetContext(inkCylinderIndex);
            Grid.SetRow(DetailsView, colorIndex * NumberOfGridRowsPerColor + 3);
            Grid.SetColumn(DetailsView, 0);
            Grid.SetColumnSpan(DetailsView, _PrintSystem.InkCylindersPerColor);
            DetailsView.Margin = new Thickness(5, 0, 5, 10);
            DetailsView.Visibility = System.Windows.Visibility.Visible;
            OverviewPanel.Children.Add(DetailsView);
        }

        private void DrawGridLines(int colorIndex, int inkSupplyIndex, bool buttonChecked)
        {
            if (colorIndex < 0 || colorIndex >= _PrintSystem.ColorCnt ||
                inkSupplyIndex < 0 || inkSupplyIndex >= _PrintSystem.InkCylindersPerColor)
            {
                // todo error handling
                return;
            }
            for (int i = 0; i < _PrintSystem.ColorCnt; i++)
            {
                try
                {
                    OverviewPanel.RowDefinitions[2 + NumberOfGridRowsPerColor * i].Height = new GridLength(0);
                }
                catch (Exception e)
                {
                    // todo: error handling! (programming error)
                    Console.WriteLine(e.Message);
                    return;
                }
            }

            for (int index = OverviewPanel.Children.Count - 1; index >= 0; index--)
            {
                var elem = OverviewPanel.Children[index];
                if (elem.GetType() == typeof(Border))
                {
                    var toBeDeleted = elem as UIElement;
                    OverviewPanel.Children.Remove(toBeDeleted);
                }
            }

            if (buttonChecked)
            {
                try
                {
                    OverviewPanel.RowDefinitions[2 + NumberOfGridRowsPerColor * colorIndex].Height = new GridLength(10);
                }
                catch (Exception e)
                {
                    // todo: error handling! (programming error)
                    Console.WriteLine(e.Message);
                    return;
                }

                Border topBorder = new Border();
                topBorder.BorderBrush = Brushes.WhiteSmoke;
                topBorder.BorderThickness = new Thickness(2, 0, 2, 0);
                Grid.SetRow(topBorder, 1 + colorIndex * NumberOfGridRowsPerColor);
                Grid.SetColumn(topBorder, inkSupplyIndex);
                Grid.SetRowSpan(topBorder, 2);
                if (inkSupplyIndex != 0) topBorder.Margin = new Thickness(10, 5, 0, 0);
                else topBorder.Margin = new Thickness(0, 5, 0, 0);
                OverviewPanel.Children.Add(topBorder);
                int colSpanLeft = inkSupplyIndex;

                if (colSpanLeft > 0)
                {
                    Border leftBorder = new Border();
                    Grid.SetRow(leftBorder, 3 + colorIndex * NumberOfGridRowsPerColor);
                    Grid.SetColumn(leftBorder, 0);
                    Grid.SetColumnSpan(leftBorder, colSpanLeft + 1);
                    double width = OverviewPanel.ColumnDefinitions[inkSupplyIndex].ActualWidth;
                    leftBorder.Margin = new Thickness(0, 0, width - 10, 0);
                    leftBorder.BorderBrush = Brushes.WhiteSmoke;
                    leftBorder.BorderThickness = new Thickness(2, 2, 0, 2);
                    leftBorder.CornerRadius = new CornerRadius(0, 0, 0, 10);
                    OverviewPanel.Children.Add(leftBorder);
                }
                int colSpanRight = _PrintSystem.InkCylindersPerColor - inkSupplyIndex - 1;
                if (colSpanRight > 0)
                {
                    Border rightBorder = new Border();
                    Grid.SetRow(rightBorder, 3 + colorIndex * NumberOfGridRowsPerColor);
                    Grid.SetColumn(rightBorder, inkSupplyIndex + 1);
                    Grid.SetColumnSpan(rightBorder, colSpanRight);
                    rightBorder.BorderBrush = Brushes.WhiteSmoke;
                    rightBorder.BorderThickness = new Thickness(0, 2, 2, 2);
                    rightBorder.CornerRadius = new CornerRadius(0, 0, 10, 0);
                    OverviewPanel.Children.Add(rightBorder);
                }
                Border centerBorder = new Border();
                Grid.SetRow(centerBorder, 3 + colorIndex * NumberOfGridRowsPerColor);
                Grid.SetColumn(centerBorder, inkSupplyIndex);
                centerBorder.BorderBrush = Brushes.WhiteSmoke;
                double leftThickness = colSpanLeft > 0 ? 0 : 2;
                double rightThickness = colSpanRight > 0 ? 0 : 2;
                centerBorder.BorderThickness = new Thickness(leftThickness, 0, rightThickness, 2);
                OverviewPanel.Children.Add(centerBorder);
            }
        }

        private void PrinterSettings_Clicked(object sender, RoutedEventArgs e)
        {
            PrintSystemSettings dialog = new PrintSystemSettings();
             dialog.ShowDialog();
        }

        private void _timer_Tick(int tickNo)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_FLUID_STAT);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_HEAD_STAT);
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ENCODER_STAT);

            for (int i = 0; i < _PrintSystem.InkCylindersPerColor * _PrintSystem.ColorCnt; i++)
            {
                UpdateInkCylinderStatus(i);
            }
            UpdatePrintHeadStatus();
        }

        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.Stepper.SendStepperCfg();
            _PrintSystem.SendMsg(TcpIp.CMD_SET_PRINTER_CFG);
        }
    }
}
