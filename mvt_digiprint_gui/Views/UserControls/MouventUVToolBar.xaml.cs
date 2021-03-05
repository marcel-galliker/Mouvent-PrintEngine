using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for MouventUVToolBar.xaml
    /// </summary>
    public partial class MouventUVToolBar : UserControl
    {
        public MouventUVToolBar()
        {
            InitializeComponent();
            UV_Grid.DataContext = RxGlobals.UvLamp;
        }

        //--- UV_Clicked ------------------------------------------------------------------------
        private void UV_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxGlobals.UvLamp.On || RxGlobals.UvLamp.Ready)
            {
                if ((RxGlobals.UvLamp.ActionStr == null || !RxGlobals.UvLamp.ActionStr.Equals("OFF")) && MvtMessageBox.YesNo(RX_DigiPrint.Resources.Language.Resources.UvDryer, RX_DigiPrint.Resources.Language.Resources.SwitchOff, MessageBoxImage.Question, false))
                {
                    RxGlobals.UvLamp.SwitchOff();
                }
            }
            else if (!RxGlobals.UvLamp.Busy) RxGlobals.UvLamp.SwitchOn();
        }
    }
}
