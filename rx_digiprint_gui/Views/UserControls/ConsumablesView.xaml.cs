using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using RX_DigiPrint.Services;
using RX_Common;

namespace RX_DigiPrint.Views.UserControls
{
    public partial class ConsumablesView : UserControl
    {
        public ConsumablesView()
        {
            InitializeComponent();

            for (int i=0; i<TcpIp.InkSupplyCnt+2; i++)
            {
                MainGrid.ColumnDefinitions.Add(new ColumnDefinition(){Width=GridLength.Auto});
                InkLevel ctrl = new InkLevel(){DataContext = RxGlobals.InkSupply.List[i]};
                Grid.SetColumn(ctrl, i);
                MainGrid.Children.Add(ctrl);
            }
        }
    }
}
