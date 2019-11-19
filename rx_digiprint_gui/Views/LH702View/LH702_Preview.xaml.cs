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

namespace RX_DigiPrint.Views.LH702View
{
    /// <summary>
    /// Interaction logic for LH702_preview.xaml
    /// </summary>
    public partial class LH702_Preview : UserControl
    {   
        private int _selected;

        public LH702_Preview()
        {
            InitializeComponent();
        }
        //--- Property ImgSettings ---------------------------------------
        private LH702_ImgSettings _ImgSettings;
        public LH702_ImgSettings  ImgSettings
        {
            set { _ImgSettings = value; }
        }

        public void UpdateSettings()
        {
            switch(_selected)
            {
            case 1: _ImgSettings.DataContext = DataContext; break;
            case 2: _ImgSettings.DataContext = _Next;       break;
            }
            _ImgSettings.CanDelete = (_selected==2);
        }
     
        //--- Property Next ---------------------------------------
        private PrintQueueItem _Next;
        public PrintQueueItem Next
        {
            set 
            {
                NextImage.DataContext = _Next = value;
                if (_Next==null)
                {
                    NextBorder.Visibility = NextImage.Visibility = Visibility.Collapsed;
                    if (_selected==2) Image_Clicked(null, null);
                }
                else 
                    NextBorder.Visibility = NextImage.Visibility = Visibility.Visible;
            }
        }
        
        //--- Property Source ------------------------------------------
        public ImageSource Source
        {
            get { return (ImageSource)GetValue(SourceProperty); }
            set { SetValue(SourceProperty, value); }
        }
        public static readonly DependencyProperty SourceProperty = DependencyProperty.Register("Source", typeof(ImageSource), typeof(LH702_Preview));

        //--- UserControl_SizeChanged ---------------------------------------------------------
        private void UserControl_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            //--- clipping ------------------------------------------------------
            double w=e.NewSize.Width;
            double h=e.NewSize.Height;
            double top=0;
            double r=(h-2*top)/4;

            PathFigure path = new PathFigure();
            path.StartPoint = new Point(0,top);
            path.Segments.Add(new LineSegment  (new Point(w-r,top),  true));
            path.Segments.Add(new BezierSegment(new Point(w+r,top+r), new Point(w-2*r,top+3*r), new Point(w, h-top), true));
            path.Segments.Add(new LineSegment  (new Point( r, h-top), true));
            path.Segments.Add(new BezierSegment(new Point(-r, top+3*r), new Point(2*r, top+r), new Point(0, top), true));

            PathGeometry geometry = new PathGeometry();
            geometry.Figures.Add(path);

            MainGrid.Clip = geometry;

//          Image.Height = ActualHeight-50;
            Image.Width     = ActualHeight-50;
            NextImage.Width = ActualHeight-50;

            //--- direction --------------------------------------------------------
            Canvas.SetLeft(Direction,  (ActualWidth-Direction.ActualWidth)/2);
            Canvas.SetLeft(Direction2, (ActualWidth-Direction.ActualWidth)/2);
            Canvas.SetTop (Direction2, ActualHeight-Direction.ActualHeight);
        }

        //--- addLineH -----------------------------------------------------------------------------------
        private void addLineH(double x1, double x2, double y, double h, int number)
        {
            MainGrid.Children.Add(new Line(){X1=x1, Y1=y, X2=x1,   Y2=y-h, Stroke=Brushes.Black});
            MainGrid.Children.Add(new Line(){X1=x2, Y1=y, X2=x2,   Y2=y-h, Stroke=Brushes.Black});
            MainGrid.Children.Add(new Line(){X1=x1, Y1=y-h/2, X2=x2, Y2=y-h/2,  Stroke=Brushes.Black});
        //  TextBlock text=new TextBlock(){Text=((char)(0x80+number)).ToString(), FontFamily=new FontFamily("Wingdings") };
        //  Canvas.SetLeft(text, (x1+x2)/2-5);
        //  if (h>0) Canvas.SetTop(text, y-21);
        //  else     Canvas.SetTop(text, y+5);
        //  MainGrid.Children.Add(text);
        }

        //--- addLineV ----------------------------------------------------------------------------------
        private void addLineV(double x, double y1, double y2, int number)
        {
            MainGrid.Children.Add(new Line(){X1=x, Y1=y1, X2=x+10,   Y2=y1, Stroke=Brushes.Black});
            MainGrid.Children.Add(new Line(){X1=x, Y1=y2, X2=x+10,   Y2=y2, Stroke=Brushes.Black});
            MainGrid.Children.Add(new Line(){X1=x+5, Y1=y1, X2=x+5,  Y2=y2,  Stroke=Brushes.Black});
        //  TextBlock text=new TextBlock(){Text=((char)(0x80+number)).ToString(), FontFamily=new FontFamily("Wingdings") };
        //  Canvas.SetLeft(text, x+7);
        //  Canvas.SetTop(text, (y1+y2)/2-10);
        //  MainGrid.Children.Add(text);
        }

        private int FirstMesurementChild=0;
        private void Border_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (FirstMesurementChild>0)
            {
                while (MainGrid.Children.Count>FirstMesurementChild)
                {
                    MainGrid.Children.RemoveAt(FirstMesurementChild);
                }
            }
            FirstMesurementChild = MainGrid.Children.Count;

            double x = Canvas.GetLeft(Border)+e.NewSize.Width;
            double b = Canvas.GetTop(Border)+Border.ActualHeight;

            Canvas.SetLeft(PrintMark, x+100);
            Canvas.SetTop (PrintMark, 20);

            double left=Canvas.GetLeft(PrintMark);
            addLineV(x, b, MainGrid.ActualHeight, 1);
            addLineH(x, left-2, 30, 10, 2);

            //--- Next --------------------------------------
            Canvas.SetLeft(NextBorder, left+20);
        }

        //--- MoveUp_Clicked --------------------------------------
        private void MoveUp_Clicked(object sender, RoutedEventArgs e)
        {
            PrintQueueItem item = DataContext as PrintQueueItem;
            if (item!=null)
            {
                RxButton button = sender as RxButton;
                button.IsChecked = true;
                MoveUp.Text="";
                RxNumPad pad = new RxNumPad(MoveUp);
                if((bool)pad.ShowDialog())
                {
                    item.PageMargin += Rx.StrToDouble(pad.Result);
                    item.SendMsg(TcpIp.EVT_SET_PRINT_QUEUE);
                }
                button.IsChecked = false;
            }
        }

        //--- MoveLeft_Clicked --------------------------------------
        private void MoveLeft_Clicked(object sender, RoutedEventArgs e)
        {
            PrintQueueItem item = DataContext as PrintQueueItem;
            if (item!=null)
            {
                RxButton button = sender as RxButton;
                button.IsChecked = true;
                MoveLeft.Text="";
                RxNumPad pad = new RxNumPad(MoveLeft);
                if((bool)pad.ShowDialog())
                {
                    item.PrintGoDist += Rx.StrToDouble(pad.Result);
                    item.SendMsg(TcpIp.EVT_SET_PRINT_QUEUE);
                }
                button.IsChecked = false;
            }
        }

        //--- Change_Clicked --------------------------------------
        private void Change_Clicked(object sender, RoutedEventArgs e)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_CHG_PRINT_QUEUE);
        }

        //--- Image_Clicked --------------------------------------
        private void Image_Clicked(object sender, MouseButtonEventArgs e)
        {
            NextBorder.BorderBrush = Application.Current.Resources["RX.Green"] as Brush;
            Border.BorderBrush     = Brushes.Blue;
            _selected = 1;
            UpdateSettings();
        }

        //--- NextImage_Clicked ------------------------------------
        private void NextImage_Clicked(object sender, MouseButtonEventArgs e)
        {
            Border.BorderBrush     = Application.Current.Resources["RX.Green"] as Brush;
            NextBorder.BorderBrush = Brushes.Blue;
            _selected = 2;
            UpdateSettings();
        }

    }
}
