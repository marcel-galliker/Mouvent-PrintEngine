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
            
            RB_Main.IsChecked=true;
            Device_selected(0);
            Machine.DeviceSelected +=  Device_selected;


            foreach(var ctrl in DeviceGrid.Children)
            {
                IPlcParPanel panel = ctrl as IPlcParPanel;
                if (panel!=null) 
                {
                    ParMain.SendParameters  += panel.Send;
                    ParMain.ResetParameters += panel.Reset;
                }
            }
        }

        //--- Device_selected -------------------------------------
        private void Device_selected(int no)
        {
            int i;
            for (i=0; i<DeviceGrid.Children.Count; i++)
            {
                DeviceGrid.Children[i].Visibility = (i==no)? Visibility.Visible : Visibility.Collapsed;
            }
        }
    }
}
