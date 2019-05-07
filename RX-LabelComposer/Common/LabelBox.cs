using RX_LabelComposer.External;
using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace RX_LabelComposer.Common
{
    public class LabelBox : System.Windows.Controls.Label
    {
        public delegate void OnLabelBoxClickedDelegate(object obj);
        public static OnLabelBoxClickedDelegate OnLabelBoxClicked = null;

        public static double _Zoom;
        private static double _ResX = RX_Rip.DPI;
        private static double _ResY = RX_Rip.DPI;
        private static double _FactX;
        private static double _FactY;
        private TextBlock _Content = new TextBlock();
        private BoxProperties _BoxProperties;

        //--- SetDefaults ---------------------------------------------
        private void SetDefaults()
        {
            BorderThickness = new Thickness(1);
            Padding = new Thickness(0);
            _Content.Foreground = Brushes.White;
            _Content.MinWidth = 20;
            _showBox();
            Content = _Content;
            AllowDrop = true;
            _FactX = _ResX/25400.0 / _Zoom;
            _FactY = _ResY/25400.0 / _Zoom;
        }

        //--- Constructor --------------------------------------------------
        public LabelBox()
        {
            SetDefaults();
        }

        //--- Property Zoom -------------------------------------------
        public double Zoom
        {
            get { return _Zoom; }
            set 
            {
                _Zoom = value;
                _FactX = _ResX/25400 / _Zoom;
                _FactY = _ResY/25400 / _Zoom;
                Canvas.SetLeft(this, _X * _FactX);
                Canvas.SetTop(this, _Y * _FactY);
                Width = _W * _FactX;
                Height = _H * _FactX;
            }
        }
        
        //--- LabelBox --------------------------------------------
        public LabelBox(int x, int y, int w, int h, BoxProperties parent)
        {
            _BoxProperties = parent;
            SetDefaults();
            X = x;
            Y = y;
            W = w;
            H = h;
        }

        //--- Property X ------------------------------------
        private int _X;
        public int X
        {
            get { return _X; }
            set 
            {
                if (value != X)
                {
                    _X = value;
                    double val = _X * _FactX;
                    Canvas.SetLeft(this, val);
                }
            }
        }

        //--- Property Y ----------------------------------------
        private int _Y;
        public int Y
        {
            get { return _Y; }
            set 
            {
                if (value != _Y)
                {
                    _Y = value;
                    Canvas.SetTop(this, _Y * _FactY);
                }
            }
        }

        //--- SetScreenPos ----------------------------------------
        public void SetScreenPos(double x, double y)
        {
            X = (int)(x / _FactX);
            Y = (int)(y / _FactY);
            if (_BoxProperties != null)
            {
                _BoxProperties.PosX = _X/1000.0;
                _BoxProperties.PosY = _Y/1000.0;
            }
        }

        //--- Property W -------------------------------------------
        private int _W;
        public int W
        {
            get { return _W; }
            set 
            { 
                _W = value;
                Width = _W * _FactX;
            }
        }

        //--- Property H --------------------------------------------------
        private int _H;
        public int H
        {
            get { return _H; }
            set 
            { 
                _H = value;
                Height = _H * _FactY;
            }
        }

        //--- Property No ------------------------------------------------------
        private int _No;
        public int No
        {
            get { return _No; }
            set {
                _No = value;
                _Content.Text = Convert.ToString(value); 
            }
        }
        
        //--- Property IsSelected -----------------------------------------------
        private bool _IsSelected;
        public bool IsSelected
        {
            get { return _IsSelected; }
            set {
                _IsSelected = value;
                _showBox();
            }
        }

        //--- _showBox -------------------------------------------------------
        private void _showBox()
        {
            if (_IsSelected)
            {
                BorderBrush = Brushes.Blue;
                _Content.Visibility = Visibility.Visible;
                _Content.Background = Brushes.Blue;
                Canvas.SetZIndex(this, 1);
            }
            else if (ShowBox)
            {
                BorderBrush = Brushes.LightGray;
                _Content.Visibility = Visibility.Visible;
                _Content.Background = Brushes.LightGray;
                Canvas.SetZIndex(this, 0);
            }
            else
            {
                BorderBrush = Brushes.Transparent;
                _Content.Visibility = Visibility.Hidden;
                Canvas.SetZIndex(this, 0);
            }
        }

        //--- Property ShowBox --------------------------------------
        private bool _ShowBox;
        public bool ShowBox
        {
            get { return _ShowBox; }
            set 
            { 
                _ShowBox = value;
                _showBox();
            }
        }

        //--- OnMouseDown ---------------------------------------------
        private bool over = false;
        protected override void OnMouseDown(MouseButtonEventArgs e)
        {
            if (over)(Parent as LabelArea).MovingBox = this;
            if (OnLabelBoxClicked != null) OnLabelBoxClicked(this);
        }

        //--- Hit ----------------------------------------------------
        public bool Hit(Point pt)
        {
            double x = _X * _FactX;
            double y = _Y * _FactY;
            over = (x <= pt.X && pt.X <= x + _Content.ActualWidth && y < pt.Y && pt.Y <= y + _Content.ActualHeight);
            return over;
        }
    }
}
