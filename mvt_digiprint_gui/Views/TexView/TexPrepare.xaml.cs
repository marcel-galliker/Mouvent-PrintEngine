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
        private int _off_timer=0;

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
            int value;
            bool on;
            EnPlcState state = (EnPlcState)Rx.StrToInt32(RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_MACHINE_STATE")); 
            try
            {
                on = (state==EnPlcState.plc_prepare || state==EnPlcState.plc_pause);

                if (_off_timer>0) _off_timer--;     
                if (on || _off_timer>0)
                {
                    int n;
                    this.Visibility=Visibility.Visible;
                    value=Rx.StrToInt32(RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_PREPARE_ACTIVE"));
                    str=RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_HEAD_IS_UP");
                    if (str!=null && str.Equals("TRUE")) value |= 1<<7;
                    if (RxGlobals.PrinterStatus.DataReady) value |= 1<<8;
                    for (n=0; n<_Image.Count; n++)
                    {
                        _Image[n].Visibility = ((value & (1<<n))==0)? Visibility.Collapsed : Visibility.Visible;
                    }
                }
                else this.Visibility=Visibility.Collapsed;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }

        }
    }
}
