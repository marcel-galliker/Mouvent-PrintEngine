using RX_Common;
using RX_DigiPrint.Models;
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

namespace RX_DigiPrint.Views.LB702WBView
{
    /// <summary>
    /// Interaction logic for TexPrepare.xaml
    /// </summary>
    public partial class LB702WB_Prepare : UserControl
    {
        private List<Image> _Image    = new List<Image>();
        private static BitmapImage  _img_ok   = new BitmapImage(new Uri("../../Resources/Bitmaps/confirm.ico", UriKind.Relative));

        public LB702WB_Prepare()
        {
            InitializeComponent();

            Visibility = Visibility.Collapsed;
            int i,n;
            for (i = 0, n = 0; i < PrepGrid.Children.Count; i++)
            {
               TextBlock ctrl = PrepGrid.Children[i] as TextBlock;
                if (ctrl != null)
                {
                    if ((n & 1) != 0)
            {
                        ctrl.Background = Application.Current.Resources["XamGrid_Alternate"] as Brush;
                        Grid.SetColumnSpan(ctrl, 2);
                    }
                Image img = new Image();
                    if (n < 10) Grid.SetRow(img, n + 1);
                    else Grid.SetRow(img, n + 2);
                Grid.SetColumn(img, 1);
                img.Source = _img_ok;
                img.Height = 20;
                    img.Tag = ctrl.Tag;
                _Image.Add(img);
                    PrepGrid.Children.Add(_Image[n]);
                    n++;

                }
            }
        }

        //--- Update ----------------------------------------------
        public void Update()
        {
            string str;
            int n;
            int value = 0;
            str = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_MACHINE_STATE");
            try
            {
                // visible when state="PREPARE"(14)
                value = Rx.StrToInt32(str);
                this.Visibility = (value == 4 || value == 5) ? Visibility.Visible : Visibility.Collapsed;                         

                str = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_PREPARE_ACTIVE");

                value = Rx.StrToInt32(str);

                for (n = 0; n < _Image.Count; n++)
                {
                    if (Convert.ToInt32(_Image[n].Tag) < 16) _Image[n].Visibility = ((value & (1 << Convert.ToInt32(_Image[n].Tag))) == 0) ? Visibility.Collapsed : Visibility.Visible;
                    else if (Convert.ToInt32(_Image[n].Tag) == 16)
                    {
                        if (RxGlobals.PrinterStatus.DataReady) _Image[n].Visibility = Visibility.Visible;
                        else _Image[n].Visibility = Visibility.Collapsed;
                    }
                    else if (Convert.ToInt32(_Image[n].Tag) == 17)
                    {
                        if (RxGlobals.StepperStatus[0].Z_in_print) _Image[n].Visibility = Visibility.Visible;
                        else _Image[n].Visibility = Visibility.Collapsed;
                    }
                    else if (Convert.ToInt32(_Image[n].Tag) == 18)
                {
                        if (RxGlobals.StepperStatus[0].RefDone) _Image[n].Visibility = Visibility.Visible;
                        else _Image[n].Visibility = Visibility.Collapsed;
                    }
                }
            }
            catch(Exception)
            { }

        }
    }
}
