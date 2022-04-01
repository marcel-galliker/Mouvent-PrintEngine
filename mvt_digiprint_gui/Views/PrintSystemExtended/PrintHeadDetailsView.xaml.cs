using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.PrintSystemExtendedView
{
    /// <summary>
    /// Interaction logic for PrintHeadsView.xaml
    /// </summary>
    public partial class PrintHeadDetailsView : UserControl
    {
        private List<ClusterView> _ClusterViewList = new List<ClusterView>();
        private PrintSystem _PrintSystem = RxGlobals.PrintSystem;
        private int _NumberOfClustersPerInkCylinder;

        private double MaxColumnWidth { get; set; }

        public PrintHeadDetailsView()
        {
            InitializeComponent();
            DataContext = this;

            MaxColumnWidth = 0.0;
            _NumberOfClustersPerInkCylinder = (_PrintSystem.HeadsPerColor+TcpIp.HEAD_CNT-1)/TcpIp.HEAD_CNT;
            _NumberOfClustersPerInkCylinder /= _PrintSystem.InkCylindersPerColor;
            for (int i = 0; i < _NumberOfClustersPerInkCylinder; i++)
            {
                ClusterView clusterView = new ClusterView();
                _ClusterViewList.Add(clusterView);
            }
        }

        public void SetInkCylinderIndex(int inkCylinderIndex)
        {
            ClusterViewStack.Children.Clear();
            int color = inkCylinderIndex / _PrintSystem.InkCylindersPerColor;
            int HeadsPerInkCylinder = _PrintSystem.HeadsPerColor / _PrintSystem.InkCylindersPerColor;
            int firstPrintHeadIndexOfCylinder = color*_PrintSystem.HeadsPerColor+(inkCylinderIndex%_PrintSystem.InkCylindersPerColor)*HeadsPerInkCylinder;

            for (int i = 0; i < _NumberOfClustersPerInkCylinder; i++)
            {
                int firstPrintHeadIndex = firstPrintHeadIndexOfCylinder + i * TcpIp.HEAD_CNT;

                _ClusterViewList[i].SetFirstPrintHeadIndex(firstPrintHeadIndex);

                ClusterViewStack.Children.Add(_ClusterViewList[i]);
            }
        }

        public void SetUser()
        {
            ServiceGrid.Visibility = System.Windows.Visibility.Visible;	            
        }
    }
}
