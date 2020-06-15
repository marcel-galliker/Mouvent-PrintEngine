using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.PrintSystemExtendedView
{
    /// <summary>
    /// Interaction logic for ClusterView.xaml
    /// </summary>


    public partial class ClusterView : UserControl
    {
        private List<PrintHeadView> PrintHeadViewList = new List<PrintHeadView>();
        private int _FirstPrintHeadIndex;

        public ClusterView()
        {
            InitializeComponent();

            for (int i = 0; i < TcpIp.HEAD_CNT; i++)
            {
                PrintHeadViewList.Add(new PrintHeadView());
            }
        }

        public void SetFirstPrintHeadIndex(int printHeadIndex)
        {
            _FirstPrintHeadIndex = printHeadIndex;
            DrawCluster();
        }

        private void DrawCluster()
        {
            PrintHeadStack.Children.Clear();

            for (int i = 0; i < TcpIp.HEAD_CNT; i++ )
            {
                var view = PrintHeadViewList[i];
                int index = _FirstPrintHeadIndex + i;
                try
                {
                    view.DataContext = RxGlobals.HeadStat.List[index];
                }
                catch (Exception ex)
                {
                    // List not big enough!
                    Console.WriteLine(ex.Message);
                    return;
                }
                view.SetPrintHeadIndex(index);
                PrintHeadStack.Children.Add(view);
            }
        }
    }
}
