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

namespace RX_DigiPrint.Views.CleafView
{
    /// <summary>
    /// Interaction logic for TexPrepare.xaml
    /// </summary>
    public partial class CleafPrepare : UserControl
    {
        private List<Image> _Image    = new List<Image>();
        private static BitmapImage  _img_ok   = new BitmapImage(new Uri("../../Resources/Bitmaps/confirm.ico", UriKind.Relative));

        public CleafPrepare()
        {
            InitializeComponent();
            
            Visibility = Visibility.Collapsed;
            int i;
            for (i=0; i<PrepGrid.RowDefinitions.Count(); i++)
            {
                Image img = new Image();
                Grid.SetRow(img, i+1);
                Grid.SetColumn(img, 1);
                img.Source = _img_ok;
                img.Height = 20;
                _Image.Add(img);
                PrepGrid.Children.Add(_Image[i]);
            }
        }

        //--- Update ----------------------------------------------
        public void Update()
        {
            string str;
            int i, n;
            str = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_MACHINE_STATE");
            try
            {
                // visible when state="PREPARE(4)"
                i=Rx.StrToInt32(str);
                if (i==4) this.Visibility = Visibility.Visible;
                else      this.Visibility = Visibility.Collapsed;

                str = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_PREPARE_ACTIVE");
                i=Rx.StrToInt32(str);
                for (n=0; n<_Image.Count; n++)
                {
                    _Image[n].Visibility = ((i & (1<<n))==0)? Visibility.Collapsed : Visibility.Visible;
                }
            }
            catch(Exception)
            { }

        }
    }
}
