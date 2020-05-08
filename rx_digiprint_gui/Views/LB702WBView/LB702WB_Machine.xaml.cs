using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
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
    /// Interaction logic for LB702WB_Machine.xaml
    /// </summary>
    public partial class LB702WB_Machine : UserControl
    {
        private Rectangle[] BackRect  = new Rectangle[9];
        private Rectangle[] FrontRect = new Rectangle[9];
        private Brush[]     Fill      = new Brush[3];
        private Image[]     ChangedCtrl   = new Image[9];

        public  Action<int>      DeviceSelected;

        public LB702WB_Machine()
        {
            InitializeComponent();
            {
                int i;

                Fill[0] = Brushes.LightGreen; // (SolidColorBrush)(new BrushConverter().ConvertFrom("#1990EE90"));
                Fill[1] = Brushes.LightYellow; // (SolidColorBrush)(new BrushConverter().ConvertFrom("#19FFFFE0"));
                Fill[2] = (SolidColorBrush)(new BrushConverter().ConvertFrom("#6600FFFF"));

                for (i=0; i<FrontGrid.ColumnDefinitions.Count; i++)
                {
                    BackGrid.ColumnDefinitions.Add(new ColumnDefinition(){Width=FrontGrid.ColumnDefinitions[i].Width});
                }

                for (i=0; i<BackRect.Count(); i++)
                {
                    BackRect[i] = new Rectangle();
                    BackRect[i].Fill = Brushes.Transparent; // Fill[i%2];
                    BackRect[i].StrokeThickness = 1;
                    BackRect[i].Stroke = Brushes.Gray;
                    Grid.SetColumn(BackRect[i], i);
                    BackGrid.Children.Add(BackRect[i]);
                    
                    FrontRect[i] = new Rectangle();
                    FrontRect[i].Fill = Brushes.Transparent;
                    FrontRect[i].MouseDown += Rect_MouseDown;                    
                    Grid.SetColumn(FrontRect[i], i);
                    Grid.SetRowSpan(FrontRect[i], 10);
                    FrontGrid.Children.Add(FrontRect[i]);

                    ChangedCtrl[i] = new Image()
                    {
                        HorizontalAlignment=HorizontalAlignment.Center,  
                        VerticalAlignment=VerticalAlignment.Top, 
                        Height=20,
                        Visibility=Visibility.Collapsed
                    };
                    Grid.SetColumn(ChangedCtrl[i], i);

                    ChangedCtrl[i].Source=new BitmapImage(new Uri("../../Resources/Bitmaps/Changed.ico", UriKind.Relative));

                    FrontGrid.Children.Add(ChangedCtrl[i]);
                }
                ParPanel.Update += ParPanel_Update;
                RxGlobals.LB702WB_Machine = this;
            }
        }

        //--- Changed -----------------------------------
        public void Changed(int col, bool changed)
        {
            ChangedCtrl[col].Visibility = changed? Visibility.Visible:Visibility.Collapsed;
        }

        //--- ParPanel_Update ----------------------
        void ParPanel_Update()
        {
            string str;
            str = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "PAR_WIND_DIRECTION_WINDER1");
            if (str!=null)
            {
                if (str.Equals("1")) TX_DirUW.Text = "Q";
                else                 TX_DirUW.Text = "P"; 
            }
            str = RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "PAR_WIND_DIRECTION_WINDER2");
            if (str!=null)
            {
                if (str.Equals("1")) TX_DirRW.Text = "Q";
                else                 TX_DirRW.Text = "P"; 
            }
        }

        //--- Rect_MouseDown --------------------------------------------------
        void Rect_MouseDown(object sender, MouseButtonEventArgs e)
        {
            Rectangle r = sender as Rectangle;
            int no=0;

            if ((r == FrontRect[1]) || (r == FrontRect[2]))
            {
                if (BackRect[1].Fill == Brushes.Transparent)
                {
                    BackRect[1].Fill = Fill[2];
                    BackRect[1].Stroke = Brushes.DarkCyan;
                    BackRect[2].Fill = Fill[2];
                    BackRect[2].Stroke = Brushes.DarkCyan;
                }
                else
                {
                    BackRect[1].Fill = Brushes.Transparent;
                    BackRect[1].Stroke = Brushes.Gray;
                    BackRect[2].Fill = Brushes.Transparent;
                    BackRect[2].Stroke = Brushes.Gray;
                }
            }
            else if ((r == FrontRect[3]) || (r == FrontRect[4]) || (r == FrontRect[5]))
            {
                if (BackRect[3].Fill == Brushes.Transparent)
                {
                    BackRect[3].Fill = Fill[2];
                    BackRect[3].Stroke = Brushes.DarkCyan;
                    BackRect[4].Fill = Fill[2];
                    BackRect[4].Stroke = Brushes.DarkCyan;
                    BackRect[5].Fill = Fill[2];
                    BackRect[5].Stroke = Brushes.DarkCyan;
                }
                else
                {
                    BackRect[3].Fill = Brushes.Transparent;
                    BackRect[3].Stroke = Brushes.Gray;
                    BackRect[4].Fill = Brushes.Transparent;
                    BackRect[4].Stroke = Brushes.Gray;
                    BackRect[5].Fill = Brushes.Transparent;
                    BackRect[5].Stroke = Brushes.Gray;
                }
            }
            else if ((r == FrontRect[6]) || (r == FrontRect[7]))
            {
                if (BackRect[6].Fill == Brushes.Transparent)
                {
                    BackRect[6].Fill = Fill[2];
                    BackRect[6].Stroke = Brushes.DarkCyan;
                    BackRect[7].Fill = Fill[2];
                    BackRect[7].Stroke = Brushes.DarkCyan;
                }
                else
                {
                    BackRect[6].Fill = Brushes.Transparent;
                    BackRect[6].Stroke = Brushes.Gray;
                    BackRect[7].Fill = Brushes.Transparent;
                    BackRect[7].Stroke = Brushes.Gray;
                }
            }
            no = 0;
            if (BackRect[1].Fill == Fill[2]) no += 1;
            if (BackRect[3].Fill == Fill[2]) no += 10;
            if (BackRect[6].Fill == Fill[2]) no += 100;
            if (DeviceSelected != null) DeviceSelected(no);          
        }
    }
}
