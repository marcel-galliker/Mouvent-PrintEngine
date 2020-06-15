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

namespace RX_DigiPrint.Views.LB701UVView
{
    /// <summary>
    /// Interaction logic for TexPrepare.xaml
    /// </summary>
    public partial class LB701UV_Prepare : UserControl
    {
        private List<Image> _Image    = new List<Image>();
        private static BitmapImage  _img_ok   = new BitmapImage(new Uri("../../Resources/Bitmaps/confirm.ico", UriKind.Relative));

        public LB701UV_Prepare()
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
                    Grid.SetRow(img, i+1);
                    Grid.SetColumn(img, 1);
                    img.Source = _img_ok;
                    img.Height = 20;
                    img.Tag    = ctrl.Tag;
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
                value=Rx.StrToInt32(str);
                this.Visibility = (value==4 || value==5) ?  Visibility.Visible:Visibility.Collapsed;                        

                str = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_PREPARE_ACTIVE");
                value=Rx.StrToInt32(str);
                for (n=0; n<_Image.Count; n++)
                {
                    _Image[n].Visibility = ((value & (1<<Convert.ToInt32(_Image[n].Tag)))==0)? Visibility.Collapsed : Visibility.Visible;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }
    }
}
