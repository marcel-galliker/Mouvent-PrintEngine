using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
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

namespace RX_DigiPrint.Views.TexView
{
    /// <summary>
    /// Interaction logic for TexPrepare.xaml
    /// </summary>
    public partial class TexPrepare : UserControl
    {
        private List<Image> _Image    = new List<Image>();
        private static BitmapImage  _img_ok   = new BitmapImage(new Uri("../../Resources/Bitmaps/confirm.ico", UriKind.Relative));

        public TexPrepare()
        {
            InitializeComponent();
            
            int i;
            for (i=0; i<PrepGrid.RowDefinitions.Count(); i++)
            {
                Image img = new Image();
                Grid.SetRow(img, i+1);
                Grid.SetColumn(img, 0);
                img.Source = _img_ok;
                img.Height = 20;
                img.HorizontalAlignment = HorizontalAlignment.Left;
                _Image.Add(img);
                PrepGrid.Children.Add(_Image[i]);
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
            //    value=Rx.StrToInt32(str);
            //    this.Visibility = (value==4) ?  Visibility.Visible:Visibility.Collapsed;   
                this.Visibility = (RxGlobals.PrinterStatus.PrintState == EPrintState.ps_printing) ?  Visibility.Visible:Visibility.Collapsed;                     

                str = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_PREPARE_ACTIVE");                
                value=Rx.StrToInt32(str);
                str=RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_HEAD_IS_UP");
                if (str!=null && str.Equals("TRUE")) value |= 1<<7;
                if (RxGlobals.PrinterStatus.DataReady) value |= 1<<8;
                for (n=0; n<_Image.Count; n++)
                {
                    _Image[n].Visibility = ((value & (1<<n))==0)? Visibility.Collapsed : Visibility.Visible;
                }
            }
            catch(Exception)
            { }

        }
    }
}
