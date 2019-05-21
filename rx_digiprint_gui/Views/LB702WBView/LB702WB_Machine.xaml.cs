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
                }
                ParPanel.Update += ParPanel_Update;
            }
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
            int i;
            for (i=0; i<FrontRect.Count(); i++)
            {
                if (r==FrontRect[i]) 
                {
                    BackRect[i].Fill = Fill[2];
                    BackRect[i].Stroke = Brushes.DarkCyan;
                    if (DeviceSelected!=null) DeviceSelected(i);
                }
                else
                {
                    BackRect[i].Fill = Brushes.Transparent;
                    BackRect[i].Stroke = Brushes.Gray;
                }
            }
        }
    }
}
