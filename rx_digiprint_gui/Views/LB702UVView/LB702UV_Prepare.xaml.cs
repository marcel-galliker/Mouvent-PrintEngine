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

namespace RX_DigiPrint.Views.LB702UVView
{
    /// <summary>
    /// Interaction logic for TexPrepare.xaml
    /// </summary>
    public partial class LB702UV_Prepare : UserControl
    {
        private List<Image> _Image    = new List<Image>();
        private static BitmapImage  _img_ok   = new BitmapImage(new Uri("../../Resources/Bitmaps/confirm.ico", UriKind.Relative));

        public LB702UV_Prepare()
        {
            InitializeComponent();
            
            Visibility = Visibility.Collapsed;
            int i, n;
            for (i=0, n=0; i<PrepGrid.Children.Count; i++)
            {
                TextBlock ctrl = PrepGrid.Children[i] as TextBlock;
                if (ctrl!=null)
                {
                    if ((n&1)!=0)
                    {
                        ctrl.Background = Application.Current.Resources["XamGrid_Alternate"] as Brush;
                        Grid.SetColumnSpan(ctrl, 2);
                    }
                    Image img = new Image();
                    Grid.SetRow(img, n+1);
                    Grid.SetColumn(img, 1);
                    img.Source  = _img_ok;
                    img.Height  = 20;
                    img.Tag     = ctrl.Tag;
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
            int value, n;
            str = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_MACHINE_STATE");
            try
            {
                // visible when state="PREPARE"(4)
                //              state="PAUSE"(5)
                value=Rx.StrToInt32(str);
                this.Visibility = (value==4 || value==5) ?  Visibility.Visible:Visibility.Collapsed;                        

                str = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_PREPARE_ACTIVE");

                value=Rx.StrToInt32(str);
                if (RxGlobals.PrinterStatus.DataReady)      value |= 1<<14;
                if (RxGlobals.TestTableStatus.Z_in_print)   value |= 1<<15;
                if (RxGlobals.TestTableStatus.RefDone)      value |= 1<<16;

                for (n=0; n<_Image.Count; n++)
                {
                    _Image[n].Visibility = ((value & (1<<Convert.ToInt32(_Image[n].Tag)))==0)? Visibility.Collapsed : Visibility.Visible;
                }
            }
            catch(Exception)
            { }

        }
    }
}
