using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Views.UserControls;
using System;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.LB702WBView
{
    /// <summary>
    /// Interaction logic for LB701UVView.xaml
    /// </summary>
    public partial class LB702WBView : UserControl
    {
        public LB702WBView()
        {
            InitializeComponent();
            PlcNotConnected.DataContext = RxGlobals.Plc;

            RB_Main.IsChecked = true;
            Device_selected(0);
            Machine.DeviceSelected += Device_selected;
        }

        //--- Device_selected -------------------------------------
        private void Device_selected(int no)
        {
            int yes = no;

            if (yes >= 100)
            {
                ParMain.FlexoUnit.Visibility = Visibility.Visible;
                yes -= 100;
            }
            else ParMain.FlexoUnit.Visibility = Visibility.Collapsed;

            if (yes >= 10)
            {
                ParMain.PrintUnit.Visibility = Visibility.Visible;
                yes -= 10;
            }
            else ParMain.PrintUnit.Visibility = Visibility.Collapsed;

            if (yes >= 1)
                ParMain.CoatingUnit.Visibility = Visibility.Visible;
            else ParMain.CoatingUnit.Visibility = Visibility.Collapsed;

            ParMain.CoatingUnitSeparation.Visibility = ParMain.CoatingUnit.Visibility;
            ParMain.PrintUnitSeparation.Visibility = ParMain.PrintUnit.Visibility;
            ParMain.FlexoUnitSeparation.Visibility = ParMain.FlexoUnit.Visibility;
        }
    }
}
