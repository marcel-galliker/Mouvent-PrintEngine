using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for Preview.xaml
    /// </summary>
    public partial class PreviewScan : UserControl
    {
        private Thickness _ImageMargin = new Thickness(0,0,30,0);

        //--- constructor ------------------------------------------------
        public PreviewScan()
        {
            InitializeComponent();
            Image.DataContext=this;
            Image2.DataContext=this;
            ImageBorder.DataContext=this;
            ImageBorder2.DataContext=this;
        }
           
        //--- Property Source ------------------------------------------
        public ImageSource Source
        {
            get { return (ImageSource)GetValue(SourceProperty); }
            set { SetValue(SourceProperty, value); }
        }
        public static readonly DependencyProperty SourceProperty = DependencyProperty.Register("Source", typeof(ImageSource), typeof(PreviewScan));
        
        //--- Property Orientation --------------------------------------------
        public int Orientation
        {
            get { return (int)GetValue(OrientationProperty); }
            set { SetValue(OrientationProperty, value); }
        }
        public static readonly DependencyProperty OrientationProperty = DependencyProperty.Register("Orientation", typeof(int), typeof(PreviewScan), new PropertyMetadata(1234,  new PropertyChangedCallback(OnOrientationChanged)));

        private static void OnOrientationChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            PreviewScan p = (PreviewScan)d;
            if (p!=null) p.addMeasurement();
        }

        //--- addMeasurement ------------------------------------------------------------
        private void addMeasurement()
        {
            if (Image.ActualHeight==0) return;

            MyCanvas.Children.RemoveRange(0,MyCanvas.Children.Count);
                                
            double border = 30;
            double distY  = 20;
            double left, right, top, bottom;
            
            //--- set rectangle -----------------
            right =  MainGrid.ActualWidth-border;            
            if (Orientation==90 || Orientation==270)
            {
                _ImageMargin.Right = border;
                left   = MainGrid.ActualWidth - Image.ActualWidth - border;
                top    = (MainGrid.ActualHeight-Image.ActualHeight)/2;
                bottom = (MainGrid.ActualHeight+Image.ActualHeight)/2;
            }
            else
            {
                _ImageMargin.Right = border + (Image.ActualHeight-Image.ActualWidth)/2;
                left   = MainGrid.ActualWidth - Image.ActualHeight - border;
                top    = (MainGrid.ActualHeight-Image.ActualWidth)/2;
                bottom = (MainGrid.ActualHeight+Image.ActualWidth)/2;
            }

      //      addLineV(left-12, top,                     bottom,   -10, 1);   // height           
      //      addLineH(left,    right,                   bottom+2, -10, 2);   // width       
      //      addLineH(right,   MainGrid.ActualWidth,    bottom+2, -10, 3);   // border
      //      addLineV(right,   top,                     top-distY/2,0, 4);   // gap-Y           
            addLineH(right,   MainGrid.ActualWidth,    bottom+2, -10, 1);   // border
            addLineV(right,   top,                     top-distY/2,0, 2);   // gap-Y           
            Image.Margin = _ImageMargin;
            
            Image2.Margin= new Thickness(0, -(bottom-top)*2-distY, _ImageMargin.Right,0);
        }

        //--- addLineH -----------------------------------------------------------------------------------
        private void addLineH(double x1, double x2, double y, double h, int number)
        {
            MyCanvas.Children.Add(new Line(){X1=x1, Y1=y, X2=x1,   Y2=y-h, Stroke=Brushes.Black});
            MyCanvas.Children.Add(new Line(){X1=x2, Y1=y, X2=x2,   Y2=y-h, Stroke=Brushes.Black});
            MyCanvas.Children.Add(new Line(){X1=x1, Y1=y-h/2, X2=x2, Y2=y-h/2,  Stroke=Brushes.Black});
            TextBlock text=new TextBlock(){Text=((char)(0x80+number)).ToString(), FontFamily=new FontFamily("Wingdings") };
            Canvas.SetLeft(text, (x1+x2)/2-5);
            if (h>0) Canvas.SetTop(text, y-21);
            else     Canvas.SetTop(text, y+5);
            MyCanvas.Children.Add(text);
        }

        //--- addLineV ----------------------------------------------------------------------------------
        private void addLineV(double x, double y1, double y2, double h, int number)
        {
            MyCanvas.Children.Add(new Line(){X1=x, Y1=y1, X2=x+10,   Y2=y1, Stroke=Brushes.Black});
            MyCanvas.Children.Add(new Line(){X1=x, Y1=y2, X2=x+10,   Y2=y2, Stroke=Brushes.Black});
            MyCanvas.Children.Add(new Line(){X1=x+5, Y1=y1, X2=x+5,  Y2=y2,  Stroke=Brushes.Black});
            TextBlock text=new TextBlock(){Text=((char)(0x80+number)).ToString(), FontFamily=new FontFamily("Wingdings") };
            if (h<0)    Canvas.SetLeft(text, x+h);
            else        Canvas.SetLeft(text, x+7+h);
            Canvas.SetTop(text, (y1+y2)/2-10);
            MyCanvas.Children.Add(text);
        }

        //--- Image_SizeChanged -------------------------------------------------------------------------
        /*
        private void Image_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            Image img = sender as Image;
            if (img!=null)
            {
                if (e.NewSize.Height>e.NewSize.Width)   
                {
                    img.Width = e.NewSize.Width;
                    img.Height=90;
                }
                else                                    
                {
                    img.Height=e.NewSize.Height;
                    img.Width =90;
                }
                Image2.Width  = img.Width;
                Image2.Height = img.Height;
                addMeasurement();
            }
        }
        */

        //--- UserControl_SizeChanged ---------------------------------------------------------
        private void UserControl_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            //--- clipping ------------------------------------------------------
            double w=e.NewSize.Width;
            double h=e.NewSize.Height;
            double r=w/8;

            PathFigure path = new PathFigure();
            path.StartPoint = new Point(0, r/8);
            path.Segments.Add(new BezierSegment(new Point( 1.5*r, -0.1*r), new Point(3*r, r), new Point(w, 0), true));
            path.Segments.Add(new LineSegment  (new Point( w+100, 0),  true));
            path.Segments.Add(new LineSegment  (new Point( w+100, h-r/8),  true));
            path.Segments.Add(new LineSegment  (new Point( w,     h-r/8),  true));
            path.Segments.Add(new BezierSegment(new Point( 2*r, h+r/4), new Point( 1.5*r, h-r/2), new Point(0, h-r/8), true));

            PathGeometry geometry = new PathGeometry();
            geometry.Figures.Add(path);

            MainGrid.Clip = geometry;
        }

        //--- MyCanvas_Loaded ---------------------------------------
        private void MyCanvas_Loaded(object sender, RoutedEventArgs e)
        {
            addMeasurement();
        }
    }
}
