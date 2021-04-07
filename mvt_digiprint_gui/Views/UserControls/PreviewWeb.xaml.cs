using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;
using static RX_DigiPrint.Services.TcpIp;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for Preview.xaml
    /// </summary>
    public partial class PreviewWeb : UserControl
    {
        private int FirstMesurementChild;
        private bool _cleaf=false;

        public PreviewWeb()
        {
            InitializeComponent();
            Image.DataContext=this;
            ImageBorder.DataContext=this;
            Image2.DataContext=this;
            ImageBorder2.DataContext=this;
            _cleaf = RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf; 
            if (_cleaf)
            {
                Direction.Text=Direction2.Text="ï ï ï";                           
            }
        }

        //--- Property ImageX ---------------------------------------
        private double _ImageX=110;
        public double ImageX
        {
            set 
            { 
                if (value!=_ImageX)
				{
                    _ImageX=value; 
                    Canvas.SetLeft(Image, _ImageX);
                    Canvas.SetLeft(ImageBorder, _ImageX);
				}
            }
            get { return _ImageX; }
        }

        //--- Property ImageX2 ---------------------------------------
        private double _ImageX2=2;
        public double ImageX2
        {
            set 
            { 
                if (value!=_ImageX2)
				{
                    _ImageX2=value; 
                    Canvas.SetLeft(Image2, _ImageX2);
                    Canvas.SetLeft(ImageBorder2, _ImageX2);
				}
            }
            get { return _ImageX2; }
        }
        //--- Property ImageY ---------------------------------------
        private int _ImageY;
        public int ImageY
        {
            set { _ImageY = value;}
            get { return _ImageY; }
        }
        
        //--- Property Source ------------------------------------------
        public ImageSource Source
        {
            get { return (ImageSource)GetValue(SourceProperty); }
            set { SetValue(SourceProperty, value); }
        }
        public static readonly DependencyProperty SourceProperty = DependencyProperty.Register("Source", typeof(ImageSource), typeof(PreviewWeb));
        
        //--- Property Orientation --------------------------------------------
        public int Orientation
        {
            get { return (int)GetValue(OrientationProperty); }
            set { SetValue(OrientationProperty, value); }
        }
        public static readonly DependencyProperty OrientationProperty = DependencyProperty.Register("Orientation", typeof(int), typeof(PreviewWeb), new PropertyMetadata(1234,  new PropertyChangedCallback(OnOrientationChanged)));

        private static void OnOrientationChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            PreviewWeb p = (PreviewWeb)d;
 
            TransformGroup myTransformGroup = new TransformGroup(); 

            if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_cleaf)  
                myTransformGroup.Children.Add(new RotateTransform(){Angle = p.Orientation+270});
            else         
                myTransformGroup.Children.Add(new RotateTransform(){Angle = p.Orientation+90}); 

            p.Image.RenderTransform  = myTransformGroup;
            p.Image2.RenderTransform = myTransformGroup;

            //--- measurement ---
            if (p.FirstMesurementChild>0)
            {
                p.addMeasurement();
            }
        }

		public TcpIp.EPrintGoMode PrintGoMode
		{
			get { return (TcpIp.EPrintGoMode)GetValue(PrintGoModeProperty); }
			set { SetValue(PrintGoModeProperty,value); }
		}

		// Using a DependencyProperty as the backing store for PrintGoMode.  This enables animation, styling, binding, etc...
		public static readonly DependencyProperty PrintGoModeProperty =
			DependencyProperty.Register("PrintGoMode",typeof(TcpIp.EPrintGoMode),typeof(PreviewWeb), new PropertyMetadata(TcpIp.EPrintGoMode.PG_MODE_GAP,  new PropertyChangedCallback(OnPrintGoModeChanged)));

        private static void OnPrintGoModeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            PreviewWeb p = (PreviewWeb)d;

            //--- measurement ---
            if (p.FirstMesurementChild>0)
            {
                p.addMeasurement();
            }
        }

		//--- addMeasurement ------------------------------------------------------------
		private void addMeasurement()
        {
            //--- remove old ---
            if (FirstMesurementChild>0)
            {
                while (MainGrid.Children.Count>FirstMesurementChild)
                {
                    MainGrid.Children.RemoveAt(FirstMesurementChild);
                }
            }

            //---  add new ---
            if (Orientation==0 || Orientation==180)
            {
                addLineV(ImageX+(Image.ActualWidth+Image.ActualHeight)/2, ImageY+(Image.ActualHeight+Image.ActualWidth)/2, ActualHeight, 1);
                switch(PrintGoMode)
				{
                    case EPrintGoMode.PG_MODE_GAP:
                                        addLineH(ImageX+(Image.ActualWidth-Image.ActualHeight)/2, ImageX2+(Image.ActualWidth+Image.ActualHeight)/2, ImageY+(Image.ActualHeight-Image.ActualWidth)/2, 10, 2);
                                        break;
                    case EPrintGoMode.PG_MODE_LENGTH:
                                        addLineH(ImageX+(Image.ActualWidth+Image.ActualHeight)/2, ImageX2+(Image.ActualWidth+Image.ActualHeight)/2, ImageY+(Image.ActualHeight-Image.ActualWidth)/2, 10, 2);
                                        break;
                    case EPrintGoMode.PG_MODE_MARK:
                    case EPrintGoMode.PG_MODE_MARK_VERSO:
                    case EPrintGoMode.PG_MODE_MARK_FILTER:
                    case EPrintGoMode.PG_MODE_MARK_INV:
                    case EPrintGoMode.PG_MODE_MARK_VERSO_INV:
                    case EPrintGoMode.PG_MODE_MARK_VRT:
                        double x=ImageX+(Image.ActualWidth+Image.ActualHeight)/2+35;
                        double y=16;
                        MainGrid.Children.Add(new Line(){X1=x, Y1=y-2, X2=x, Y2=y+2, Stroke=Brushes.Red, StrokeThickness=5});
                        addLineH(ImageX+(Image.ActualWidth+Image.ActualHeight)/2, x, y, -10, 2);
                        break;
                    default: break;
				}
            }
            else
            {
            /*
                addLineV(ImageX+Image.ActualWidth, ImageY, ImageY+Image.ActualHeight, 1);            
                addLineH(ImageX, ImageX+Image.ActualWidth, ImageY+Image.ActualHeight, -10, 2);
                addLineV(ImageX+Image.ActualWidth, ImageY, 0, 3);
                addLineH(ImageX + Image.ActualWidth, ImageX2+Image.ActualWidth, ImageY, 10, 4);
                addLineH(ImageX + Image.ActualWidth, MainGrid.ActualWidth-35, ImageY+Image.ActualHeight, -10, 5);
                MainGrid.Children.Add(new Line(){X1=MainGrid.ActualWidth-35-2, Y1=ImageY+Image.ActualHeight-5, X2=MainGrid.ActualWidth-35+2, Y2=ImageY+Image.ActualHeight-5, Stroke=Brushes.Black, StrokeThickness=5});
             */
            }
        }

        //--- addLineH -----------------------------------------------------------------------------------
        private void addLineH(double x1, double x2, double y, double h, int number)
        {
            MainGrid.Children.Add(new Line(){X1=x1, Y1=y, X2=x1,   Y2=y-h, Stroke=Brushes.Black});
            MainGrid.Children.Add(new Line(){X1=x2, Y1=y, X2=x2,   Y2=y-h, Stroke=Brushes.Black});
            MainGrid.Children.Add(new Line(){X1=x1, Y1=y-h/2, X2=x2, Y2=y-h/2,  Stroke=Brushes.Blue});
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

        //--- Image_SizeChanged -------------------------------------------------------------------------
        private void Image_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            ImageX = ActualWidth-150;
            if (FirstMesurementChild==0) 
            { 
                FirstMesurementChild = MainGrid.Children.Count;
                if (e.NewSize.Height>e.NewSize.Width) 
                {
                    ImageY = 10;
                    Image2.Height = Image.Height=ActualHeight-ImageY;
                }
                else
                {
                    ImageY = 35;
                    Image2.Width = Image.Width=ActualHeight-30;
                }
            }
            ImageX2  = ImageX - Image.ActualWidth - 30;
            Canvas.SetTop(Image, ImageY);
            Canvas.SetTop(ImageBorder, ImageY);      
            Canvas.SetTop(Image2, ImageY);
            Canvas.SetTop(ImageBorder2, ImageY);      
            addMeasurement();
        }

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

            //--- direction --------------------------------------------------------
            Canvas.SetLeft(Direction,  (ActualWidth-Direction.ActualWidth)/2);
            Canvas.SetLeft(Direction2, (ActualWidth-Direction.ActualWidth)/2);
            Canvas.SetTop (Direction2, ActualHeight-Direction.ActualHeight);
        }
    }
}
