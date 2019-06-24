using RxWfTool.Models;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Globalization;
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

namespace RxWfTool.Views
{
    /// <summary>
    /// Interaction logic for WfGraph.xaml
    /// </summary>   
   
    public partial class WfGraph : UserControl
    {    
        private int X1;
        private int X2;
        private int Y1;
        private int Y2;
        private double RangeY=30;
        private double RangeX=2000;
        private Ellipse _Circle = new Ellipse(){}; 

        //--- Constructor ------------------------------------------------
        public WfGraph()
        {
            InitializeComponent();
            _Circle.Width=10;
            _Circle.Height=10;
            _Circle.Fill = Brushes.Red;
            _Circle.Visibility=Visibility.Hidden;
            Graph.Children.Add(_Circle);
           // Global.ListChanged += OnListChanged;
        }

        //--- Property WfDef --------------------------------------
        private WfDef _WfDef;
        public WfDef WfDef
        {
            get { return _WfDef; }
            set 
            { 
                _WfDef = value;
                _WfDef.ListChanged += ListChanged;
                DrawGraph();
            }
        }

        //--- ListChanged -------------------------------
        void ListChanged(object sender, EventArgs e)
        {
            DrawGraph();
            DrawActive();
        }

        //--- Property ActiveItem -------------------------
        private int _ActiveItem;
        public int ActiveItem
        {
            get { return _ActiveItem; }
            set 
            {   
                _ActiveItem = value;
                DrawActive();
            }
        }
        
        //--- DrawAxes --------------------------------------
        void DrawAxes()
        {
            Legend.Children.RemoveRange(1, Legend.Children.Count);

            Y1 = (int)(Legend.ActualHeight-30);
            Y2 = 30;
            X1 = 30;
            X2 = (int)(Legend.ActualWidth-30);

            Canvas.SetLeft  (Graph, X1);    
            Canvas.SetBottom(Graph, Y2);
            Graph.Width = X2-X1;
            Graph.Height= Y1-Y2;

            //--- Y-Axses ------------
            Line y_axes = new Line();
            y_axes.X1=X1;
            y_axes.Y1=Y1;
            y_axes.X2=y_axes.X1;
            y_axes.Y2=Y2-10;
            y_axes.StrokeThickness = 1;
            y_axes.Stroke = Brushes.Black;
            Legend.Children.Add(y_axes);

            {
                TextBlock txt = new TextBlock();
                txt.Foreground=Brushes.Black;
                txt.Text = "Volt";
                Canvas.SetTop(txt,  Y2-30);
                Canvas.SetLeft(txt, y_axes.X1-15);
                Legend.Children.Add(txt);          
            }

            for (int i=0; i>=-RangeY; i--)
            {
                Line l = new Line();
                l.Stroke = Brushes.Black;
                l.StrokeThickness=1;
                if (i%5==0)
                {
                    l.X1 = X1-5;
                    l.X2 = X1+2;
                }
                else
                {
                    l.X1 = X1-3;
                    l.X2 = X1;
                }
                l.Y1 = l.Y2 = Y2+(Y1 - Y2)*(-i/RangeY);
                Legend.Children.Add(l);

                if (i%5==0)
                {
                    TextBlock txt = new TextBlock();
                    txt.Foreground=Brushes.Black;
                    txt.Text = i.ToString();
                    Canvas.SetTop(txt,  l.Y1-10);
                    Canvas.SetLeft(txt, (3-txt.Text.Length)*6);
                    Legend.Children.Add(txt);      
                }
            }

            //--- X-Axes ------------------------------------------------------------
            Line x_axes = new Line();
            x_axes.X1=X1;
            x_axes.Y1=Y1;
            x_axes.X2=X2;
            x_axes.Y2=Y1;
            x_axes.StrokeThickness = 1;
            x_axes.Stroke = Brushes.Black;
            Legend.Children.Add(x_axes);

            {
                TextBlock txt = new TextBlock();
                txt.Foreground=Brushes.Black;
                txt.Text = "µs";
                Canvas.SetTop(txt,  Y1-10);
                Canvas.SetLeft(txt, X2+2);
                Legend.Children.Add(txt);          
            }

            for (int i=0; i<=RangeX; i+=80)
            {
                Line l = new Line();
                l.Stroke = Brushes.Black;
                l.StrokeThickness=1;
                if (i%800==0)
                {
                    l.Y1 = Y1-2;
                    l.Y2 = Y1+5;
                }
                else
                {
                    l.Y1 = Y1;
                    l.Y2 = Y1+2;
                }
                l.X1 = l.X2 = X1+(X2 - X1)*(i/RangeX);
                Legend.Children.Add(l);

                if (i%80==0)
                {
                    TextBlock txt = new TextBlock();
                    if (i%800!=0) txt.FontSize *= 0.8;
                    txt.Foreground=Brushes.Black;
                    txt.HorizontalAlignment = HorizontalAlignment.Center;
                    txt.Text = string.Format("{0}",  i/80);
                    Canvas.SetTop(txt,  Y1+10);
                    Canvas.SetLeft(txt, l.X1-(txt.Text.Length)*4);
                    Legend.Children.Add(txt);           
                }
            }
        }

        //--- DrawGraph ---------------------------------------------------
        private void DrawGraph()
        {
            if (_WfDef==null) return;
            ObservableCollection<WfItem> list = _WfDef.List;
            if (list==null) return;
            Graph.Children.RemoveRange(1, Graph.Children.Count);

            Polyline line = new Polyline();
            PointCollection points = new PointCollection();
            int width  = (int)Graph.ActualWidth;
            int height = (int)Graph.ActualHeight;

            line.StrokeThickness = 1;
            line.Stroke = Brushes.Black;
            points.Add(new Point((int)(0*width/RangeX), (int)(0*height/RangeY)));
            for (int i=0; i<list.Count; i++)
            {
                points.Add(new Point((int)(list[i].Position*width/RangeX), (int)(list[i].Voltage*height/RangeY)));
            }
            line.Points = points;
            Graph.Children.Add(line);
        }

        //--- DrawActive ---------------------------------------------------------------
        private void DrawActive()
        {
            if (_WfDef==null || _WfDef.List==null || _WfDef.List.Count<=_ActiveItem)
            {
                _Circle.Visibility = Visibility.Hidden;
                return;
            }
            ObservableCollection<WfItem> list = _WfDef.List;
            Canvas.SetTop (_Circle, (int)(WfDef.List[_ActiveItem].Voltage*Graph.ActualHeight/RangeY) - _Circle.Height/2);
            Canvas.SetLeft(_Circle, (int)(WfDef.List[_ActiveItem].Position*Graph.ActualWidth/RangeX) - _Circle.Width/2);
            _Circle.Visibility = Visibility.Visible;
        }

        //--- Legend_SizeChanged --------------------------------------------------------------
        private void Legend_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            DrawAxes();
        }

        //--- Graph_SizeChanged --------------------------------------------------------------
        private void Graph_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            DrawGraph();
            DrawActive();
        }
    }
}
