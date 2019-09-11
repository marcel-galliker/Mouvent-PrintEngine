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
        public LH702_Preview()
        {
            InitializeComponent();
            DataContext = RxGlobals.PrintingItem;
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

            Image.Height = ActualHeight-50;

            //--- direction --------------------------------------------------------
            Canvas.SetLeft(Direction,  (ActualWidth-Direction.ActualWidth)/2);
            Canvas.SetLeft(Direction2, (ActualWidth-Direction.ActualWidth)/2);
            Canvas.SetTop (Direction2, ActualHeight-Direction.ActualHeight);

            Canvas.SetLeft(PrintMark, w-100);
            Canvas.SetTop (PrintMark, 20);
        }

        //--- addLineH -----------------------------------------------------------------------------------
        private void addLineH(double x1, double x2, double y, double h, int number)
        {
            MainGrid.Children.Add(new Line(){X1=x1, Y1=y, X2=x1,   Y2=y-h, Stroke=Brushes.Black});
            MainGrid.Children.Add(new Line(){X1=x2, Y1=y, X2=x2,   Y2=y-h, Stroke=Brushes.Black});
            MainGrid.Children.Add(new Line(){X1=x1, Y1=y-h/2, X2=x2, Y2=y-h/2,  Stroke=Brushes.Black});
            TextBlock text=new TextBlock(){Text=((char)(0x80+number)).ToString(), FontFamily=new FontFamily("Wingdings") };
            Canvas.SetLeft(text, (x1+x2)/2-5);
            if (h>0) Canvas.SetTop(text, y-21);
            else     Canvas.SetTop(text, y+5);
            MainGrid.Children.Add(text);
        }

        //--- addLineV ----------------------------------------------------------------------------------
        private void addLineV(double x, double y1, double y2, int number)
        {
            MainGrid.Children.Add(new Line(){X1=x, Y1=y1, X2=x+10,   Y2=y1, Stroke=Brushes.Black});
            MainGrid.Children.Add(new Line(){X1=x, Y1=y2, X2=x+10,   Y2=y2, Stroke=Brushes.Black});
            MainGrid.Children.Add(new Line(){X1=x+5, Y1=y1, X2=x+5,  Y2=y2,  Stroke=Brushes.Black});
            TextBlock text=new TextBlock(){Text=((char)(0x80+number)).ToString(), FontFamily=new FontFamily("Wingdings") };
            Canvas.SetLeft(text, x+7);
            Canvas.SetTop(text, (y1+y2)/2-10);
            MainGrid.Children.Add(text);
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

            Border border = sender as Border;
            double left=Canvas.GetLeft(PrintMark);
            double x = Canvas.GetLeft(border)+e.NewSize.Width;
            double b = Canvas.GetTop(border)+border.ActualHeight;
            addLineV(x, b, MainGrid.ActualHeight, 1);
            addLineH(x, left-2, 30, 10, 2);
        }

        //--- MoveUp_Clicked --------------------------------------
        private void MoveUp_Clicked(object sender, RoutedEventArgs e)
        {
            RxButton button = sender as RxButton;
            button.IsChecked = true;
            MoveUp.Text="";
            RxNumPad pad = new RxNumPad(MoveUp);
            if((bool)pad.ShowDialog())
            {
                RxGlobals.PrintingItem.PageMargin += Rx.StrToDouble(pad.Result);
                RxGlobals.PrintingItem.SendMsg(TcpIp.EVT_SET_PRINT_QUEUE);
            }
            button.IsChecked = false;
        }

        //--- MoveDown_Clicked --------------------------------------
        private void MoveDown_Clicked(object sender, RoutedEventArgs e)
        {
            RxButton button = sender as RxButton;
            button.IsChecked = true;
            MoveDown.Text="";
            RxNumPad pad = new RxNumPad(MoveDown);
            if((bool)pad.ShowDialog())
            {
                RxGlobals.PrintingItem.PageMargin -= Rx.StrToDouble(pad.Result);
                RxGlobals.PrintingItem.SendMsg(TcpIp.EVT_SET_PRINT_QUEUE);
            }
            button.IsChecked = false;
        }

        //--- MoveLeft_Clicked --------------------------------------
        private void MoveLeft_Clicked(object sender, RoutedEventArgs e)
        {
            RxButton button = sender as RxButton;
            button.IsChecked = true;
            MoveLeft.Text="";
            RxNumPad pad = new RxNumPad(MoveLeft);
            if((bool)pad.ShowDialog())
            {
                RxGlobals.PrintingItem.PrintGoDist += Rx.StrToDouble(pad.Result);
                RxGlobals.PrintingItem.SendMsg(TcpIp.EVT_SET_PRINT_QUEUE);
            }
            button.IsChecked = false;
        }

        //--- MoveRight_Clicked ---------------------------------------
        private void MoveRight_Clicked(object sender, RoutedEventArgs e)
        {
            RxButton button = sender as RxButton;
            button.IsChecked = true;
            MoveRight.Text="";
            RxNumPad pad = new RxNumPad(MoveRight);
            if((bool)pad.ShowDialog())
            {
                RxGlobals.PrintingItem.PrintGoDist -= Rx.StrToDouble(pad.Result);
                RxGlobals.PrintingItem.SendMsg(TcpIp.EVT_SET_PRINT_QUEUE);
            }
            button.IsChecked = false;
        }

    }
}
