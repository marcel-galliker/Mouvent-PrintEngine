using Infragistics.Windows.DataPresenter;
using Microsoft.Win32;
using RX_Common;
using RX_LabelComposer.Common;
using RX_LabelComposer.Converters;
using RX_LabelComposer.External;
using RX_LabelComposer.Models;
using RX_LabelComposer.ViewModels;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace RX_LabelComposer.Views
{
    public partial class LayoutView : UserControl
    {
        public String Title => "Label";

        #region Inner DataContext
        public LabelPropertiesViewModel LabelPropertiesVM { get; private set; }

        #endregion

        private FieldsPage   _FieldsPage    = new FieldsPage();
        private List<Image>  _LabelImg      = new List<Image>();
        private List<Image>  _ColorImg      = new List<Image>();
        private List<Image>  _VarDataImg    = new List<Image>();

        private BitmapImage  _LabelImgSource;
        private BitmapImage  _ColorImgSource;

        //--- Constructor ---------------------------------------------------
        public LayoutView()
        {
            InitializeComponent();
        }     

        //--- Page_Loaded ----------------------------------------------------------
        private void Page_Loaded(object sender, RoutedEventArgs e)
        {
            //--- field toolbar commands ------------------------------------
            BoxInsBeforeCommand = new RxCommand(OnBoxInsBefore);
            BoxInsAfterCommand  = new RxCommand(OnBoxInsAfter);
            BoxRemoveCommand    = new RxCommand(OnBoxRemove);

            //--- Configure View Model
            this.LabelPropertiesVM = new LabelPropertiesViewModel(this.LayoutDef)
            {
                ShowLayersAction = this.show_layers
            };

            //--- Data Context -----------------------------------
            this.DataContext                    = this;
            this.LayoutProperties.DataContext   = LayoutDef;
            this.LabelProperties.DataContext    = this.LabelPropertiesVM;
            //this.BoxDetails.DataContext         = LayoutDef; SAMUEL OLD
            this.BoxPropertiesView.BoxDetails.DataContext = LayoutDef; // SAMUEL NEW
            this.BoxListToolbar.DataContext     = this;
            this.TabItem_Data.DataContext       = this;
//            this.ColorImg.DataContext           = LayoutDef;
            this.LabelAera.DataContext          = LayoutDef;

            //_LayoutDef.ContentBox = this.ContentBox; SAMUEL
            _LayoutDef.ContentBox = this.BoxPropertiesView.ContentBox;

            _LayoutDef.LabelSizeChanged             += labelSizeChanged;
            _FieldsPage.FieldSelected               += _LayoutDef.OnFieldSelected;
            CountersPage.FieldSelected              += _LayoutDef.OnFieldSelected;
            RecordNo.The_RecordNo.OnRecNoChanged    += _FileDef.RecordActivated;
            RecordNo.The_RecordNo.OnRecNoChanged    += CountersPage.ActiveRecNoChanged;
            
            Res_1_1.IsChecked = true;
            _ActiveZoom = Res_1_1 as UIElement;
            ShowBoxBorders.IsChecked = _LayoutDef.ShowBoxBorders;

            draw_ruler();
            LayoutDef.LabelArea = LabelAera as LabelArea;
            LayoutDef.BoxListGrid = BoxDefGrid;
            LabelProperties.IsExpanded = RxGlobals.Settings.LabelPropExpanded;
            BoxProperties.IsExpanded   = RxGlobals.Settings.LabelBoxPropExpanded;

            _LayoutDef.PropertyChanged += PropertyChanged;

            UpdateLabelControls(_LayoutDef.Label!=null && !_LayoutDef.Label.Equals(""));
        }

        //--- LabelImg_Update ---------------------------------------------------------------
        private void LabelImg_Update()
        {
            int i;
            for (i=0; i<_LabelImg.Count(); i++)
            {
                _LabelImg[i].Source = _LabelImgSource;
                _LabelImg[i].Width  = _LayoutDef.LabelImgWidth;
                _LabelImg[i].Height = _LayoutDef.LabelImgHeight;
                Canvas.SetLeft(_LabelImg[i], _LayoutDef.ColumnDist*i/ 25.4 * RX_Rip.DPI / _LayoutDef.Zoom);
            }
        }

        //--- ColorImg_Update ---------------------------------------------------------
        private void ColorImg_Update()
        {
            int i;
            for (i=0; i<_ColorImg.Count(); i++)
            {
                _ColorImg[i].Source = _ColorImgSource;
                _ColorImg[i].Width  = _LayoutDef.LabelImgWidth;
                _ColorImg[i].Height = _LayoutDef.LabelImgHeight;
                Canvas.SetLeft(_ColorImg[i], _LayoutDef.ColumnDist*i/ 25.4 * RX_Rip.DPI / _LayoutDef.Zoom);
            }
        }

        //--- VarData_Update ------------------------------------------------------------------
        private void VarData_Update()
        {
            int i;
            for(i=0; i<_VarDataImg.Count(); i++)
            {
                if (_LayoutDef.LabelBitmap.Count()>i) _VarDataImg[i].Source  = _LayoutDef.LabelBitmap[i];
                _VarDataImg[i].Width   = _LayoutDef.LabelBmpWidth;
                _VarDataImg[i].Height  = _LayoutDef.LabelBmpHeight;
                Canvas.SetLeft(_VarDataImg[i], _LayoutDef.ColumnDist*i/ 25.4 * RX_Rip.DPI / _LayoutDef.Zoom);
            }
        }

        //--- UpdateLabelControls --------------------------------------------------------------
        private void UpdateLabelControls(bool labelUsed)
        {
            if (!labelUsed)
            {
                /*
                LabelVisible.IsChecked    = false;
                ColorVisible.IsChecked    = false;
                VariableVisible.IsChecked = true;
                */
                this.LabelPropertiesVM.IsLabelControlChecked    = false;
                this.LabelPropertiesVM.IsColorControlChecked    = false;
                this.LabelPropertiesVM.IsVariableControlChecked = false;
                _LayoutDef.ColorLayer     = "";
            }
            // TODO: What does he want?
            /*
            LabelVisible.IsEnabled    = labelUsed;
            ColorVisible.IsEnabled    = labelUsed;
            VariableVisible.IsEnabled = labelUsed;
            ColorName.IsEnabled       = labelUsed;
            ColorButton.IsEnabled     = labelUsed;
            */
            this.LabelPropertiesVM.IsLabelControlEnabled    = labelUsed;
            this.LabelPropertiesVM.IsColorControlEnabled    = labelUsed;
            this.LabelPropertiesVM.IsVariableControlEnabled = labelUsed;
            show_layers(null, null);
        }

        //--- PropertyChanged ------------------------------------------------------------------
        void PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            //--- LABEL File -----------------------------------
            if (e.PropertyName.Equals("Label"))
            {
                object path = new LabelPath_Converter().Convert(_LayoutDef.Label, null, null, null);
                try
                {
                    if (path==null) _LabelImgSource = null;
                    else            _LabelImgSource = new BitmapImage(new Uri(path  as string));
                }
                catch (Exception)
                {
                    _LabelImgSource = null;                
                }
                /*
                LabelVisible.IsChecked = true;
                */
                this.LabelPropertiesVM.IsLabelControlChecked = true;
            }
            else if (e.PropertyName.Equals("LabelImgWidth") || e.PropertyName.Equals("LabelImgHeight")) 
            {
                LabelImg_Update();
                ColorImg_Update();
            }

            else if (e.PropertyName.Equals("LabelUnused"))
            {
                UpdateLabelControls(!_LayoutDef.LabelUnused);
            }

            //--- Color level File -----------------------------------
            else if (e.PropertyName.Equals("ColorLayer"))
            {
                try 
                {
                    _ColorImgSource = new BitmapImage(new Uri(new LabelPath_Converter().Convert(_LayoutDef.ColorLayer, null, null, null)  as string));
                    /*
                    ColorVisible.IsChecked=true;
                    */
                    this.LabelPropertiesVM.IsColorControlChecked = true;
                }
                catch(Exception)
                {
                    /*
                    ColorVisible.IsChecked=false;
                    */
                    this.LabelPropertiesVM.IsColorControlChecked = false;
                    _ColorImgSource = null;
                }
                ColorImg_Update();
                show_layers(null, null);
            }

            //--- LABEL Bitmap --------------------------------------------------
            else if (e.PropertyName.Equals("LabelBitmap") || e.PropertyName.Equals("LabelBmpWidth") || e.PropertyName.Equals("LabelBmpHeight")) 
            {
                VarData_Update();
                show_layers(null, null);
            }

            //--- Columns ----------------------------------------------------------
            else if (e.PropertyName.Equals("ColumnDist") || e.PropertyName.Equals("Zoom"))
            {
                LabelImg_Update();
                ColorImg_Update();
                VarData_Update();
            }
            else if (e.PropertyName.Equals("Columns"))
            {
                Images.Children.Clear();
                _LabelImg.Clear();
                _ColorImg.Clear();
                _VarDataImg.Clear();
                for (int i=0; i<_LayoutDef.Columns; i++)
                {
                    _LabelImg.Add(new Image());
                    Images.Children.Add(_LabelImg[i]);
                    _ColorImg.Add(new Image());
                    Images.Children.Add(_ColorImg[i]);
                    _VarDataImg.Add(new Image());
                    Images.Children.Add(_VarDataImg[i]);
                }
                VarData_Update();
                LabelImg_Update();
                ColorImg_Update();
                show_layers(null, null);
            }
            else if (e.PropertyName.Equals("WebWidth"))
            {
                draw_ruler();
            }
        }      

        //--- Page_Unloaded -------------------------------------------------------------------
        private void Page_Unloaded(object sender, RoutedEventArgs e)
        {
            _FieldsPage.FieldSelected -= _LayoutDef.OnFieldSelected;
        }

        //--- Property LayoutDef -------------------------------------------------
        private LayoutDefinition _LayoutDef;
        public LayoutDefinition LayoutDef
        {
            get { return _LayoutDef; }
            set { _LayoutDef = value; }
        }

        //--- Property FileDef -------------------------------------------------
        private FileDefinition _FileDef;
        public FileDefinition FileDef
        {
            get { return _FileDef; }
            set
            {
                _FileDef = value;
                _FieldsPage.FileDef = value;
            }
        }

        //--- Property CounterDef ---------------------------------------
        private CounterDefinition _CounterDef;
        public CounterDefinition CounterDef
        {
            get { return _CounterDef; } 
            set 
            { 
                _CounterDef = value;
                CountersPage.CounterDef = value;
            }
        }
        
        //--- ActiveRecNoChanged --------------------------------------
        public void ActiveRecNoChanged(int recNo)
        {
            _LayoutDef.ActiveRecNoChanged(recNo);
        }

        //--- Property FieldsPage ---------------------------------------------
        public FieldsPage FieldsPage
        {
            get { return _FieldsPage; }
        }
        
        //--- Box Toolbar commands -------------------------------------------
        #region box toolbar commands

        public ICommand BoxInsBeforeCommand { get; private set; }
        private void OnBoxInsBefore(object parameter)
        {
            Record selectedRecord = BoxDefGrid.ActiveRecord;
            int index;
            if (selectedRecord != null)
            {
                index = selectedRecord.Index;
            }
            else
            {
                index = 0;
            }
            this.InsertBoxAtIndex(index);
        }
 
        //--- OnBoxInsAfter ---------------------------------------------
        public ICommand BoxInsAfterCommand { get; private set; }
        private void OnBoxInsAfter(object parameter)
        {
            Record selectedRecord = BoxDefGrid.ActiveRecord;
            int index;
            if (selectedRecord != null)
            {
                index = selectedRecord.Index + 1;
            }
            else
            {
                index = this.LayoutDef.BoxList == null ? 0 : this.LayoutDef.BoxList.Count;
            }
            this.InsertBoxAtIndex(index);
        }

        private void InsertBoxAtIndex(int index)
        {
            this.LayoutDef.BoxListInsertAtIndex(index);

            //--- force refresh
            this.BoxDefGrid.DataContext  = null;
            this.BoxDefGrid.DataContext  = this.LayoutDef;
            this.BoxDefGrid.ActiveRecord = this.BoxDefGrid.Records[index];
        }

        //--- OnBoxRemove -------------------------------------------
        public ICommand BoxRemoveCommand { get; private set; }
        private void OnBoxRemove(object parameter)
        {
            Record r = BoxDefGrid.ActiveRecord;

            if (r != null)
            {
                string str = string.Format("Delete the box?");
                if (System.Windows.MessageBox.Show(str, "Delete", MessageBoxButton.YesNo, MessageBoxImage.Question, MessageBoxResult.No) == MessageBoxResult.Yes)
                {
                    LayoutDef.BoxListRemove(r.Index);
                }
            }
        }
       #endregion

        //--- Label_Clicked -------------------------------
        /* DID MOVE
        private void Label_Clicked(object sender, RoutedEventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();

            // dlg.InitialDirectory = "c:\\";
            dlg.Filter = "bmp files (*.bmp)|*.bmp|All files (*.*)|*.*";
            dlg.FilterIndex = 1;
            dlg.RestoreDirectory = true;

            if (dlg.ShowDialog() == true)
                 LayoutDef.Label = dlg.FileName;
        }
        */
        //--- ColorLayer_Clicked -------------------------------
        /* DID MOVE
        private void ColorLayer_Clicked(object sender, RoutedEventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();

            // dlg.InitialDirectory = "c:\\";
            dlg.Filter = "bmp files (*.bmp)|*.bmp|All files (*.*)|*.*";
            dlg.FilterIndex = 1;
            dlg.RestoreDirectory = true;

            if (dlg.ShowDialog() == true)
                 LayoutDef.ColorLayer = dlg.FileName;
        }
        */
        //--- BoxDefGrid_RecordActivated ---------------------------------------
        private void BoxDefGrid_RecordActivated(object sender, Infragistics.Windows.DataPresenter.Events.RecordActivatedEventArgs e)
        {
            _LayoutDef.ActiveBoxNo = e.Record.Index;
        }

        //--- LabelProperties_Expanded ------------------------------------------
        private void LabelProperties_Expanded(object sender, RoutedEventArgs e)
        {
            RxGlobals.Settings.LabelPropExpanded = (sender as Expander).IsExpanded;
        }
        
        //--- BoxProperties_Expanded ------------------------------------------
        private void BoxProperties_Expanded(object sender, RoutedEventArgs e)
        {
            RxGlobals.Settings.LabelBoxPropExpanded = (sender as Expander).IsExpanded;
        }

        //--- draw_ruler --------------------------------------------------------------
        private void draw_ruler()
        {
            int x, n, n0;

            if (_LayoutDef==null) 
                return;

            //--- horizontal ruler ----------------------
            RulerH.Children.RemoveRange(0, RulerH.Children.Count);
            n0 = (int)LabelFrame.HorizontalOffset;
            for (x = 0; x <= _LayoutDef.WebWidth; x++)
            {
                n = (int)((double)x / 25.4 * RX_Rip.DPI / _LayoutDef.Zoom);
                if (n < n0) continue;
                n -= n0;
                if (n > RulerH.ActualWidth) break;
                Line line = new Line();
                line.Stroke = Brushes.Blue;
                line.X1 = n;
                line.X2 = n;
                line.Y1 = 2;
                if (x % 10 == 0)
                {
                    TextBlock text = new TextBlock();
                    text.FontSize = 10;
                    text.Text = x.ToString();
                    Canvas.SetLeft(text, n+2);
                    Canvas.SetTop(text, 2);
                    RulerH.Children.Add(text);
                    line.Y2 = 10;
                }
                else if (x % 10 == 5) line.Y2 = 7;
                else line.Y2 = 4;
                RulerH.Children.Add(line);
            }

            //--- vertical ruler ----------------------
            RulerV.Children.RemoveRange(0, RulerV.Children.Count);
            n0 = (int)LabelFrame.VerticalOffset;
            for (x = 0; x <= _LayoutDef.LabelHeight; x++)
            {
                n = (int)((double)x / 25.4 * RX_Rip.DPI / _LayoutDef.Zoom);
                if (n < n0) continue;
                n -= n0;
                if (n > RulerV.ActualHeight) break;
                Line line = new Line();
                line.Stroke = Brushes.Blue;
                line.Y1 = n;
                line.Y2 = n;
                line.X1 = 0;
                if (x % 10 == 0)
                {
                    TextBlock text = new TextBlock();
                    text.FontSize = 10;
                    text.Text = x.ToString();
                    Canvas.SetLeft(text,2);
                    Canvas.SetTop(text, n+2);
                    RulerV.Children.Add(text);
                    line.X2 = 8;
                }
                else if (x % 10 == 5) line.X2 = 5;
                else line.X2 = 2;
                RulerV.Children.Add(line);
            }

        }

        private UIElement _ActiveZoom;

        //--- zoom_changed --------------------------------------------------------
        private void zoom_changed(object active, double zoom)
        {
            if (_LayoutDef != null) _LayoutDef.Zoom = zoom;
            _ActiveZoom = active as UIElement;
            foreach (UIElement button in ZoomToolbar.Children )
            {
                (button as ToggleButton).IsChecked = (button==_ActiveZoom);
            }
            draw_ruler();
        }

        //--- Res_1_2_Click ---------------------------------------------------------
        private void Res_1_2_Click(object sender, RoutedEventArgs e)
        {            
            zoom_changed(sender, 2 * RX_Rip.DPI / _LayoutDef.ScreenDpiX);
        }

        //--- Res_1_1_Click ----------------------------------------------------
        private void Res_1_1_Click(object sender, RoutedEventArgs e)
        {
            zoom_changed(sender, RX_Rip.DPI / _LayoutDef.ScreenDpiX);
        }

        //--- Res_2_1_Click ----------------------------------------------------
        private void Res_2_1_Click(object sender, RoutedEventArgs e)
        {
            zoom_changed(sender, RX_Rip.DPI / (2*_LayoutDef.ScreenDpiX));
        }

        //--- Res_pixel_Click -------------------------------------------
        private void Res_pixel_Click(object sender, RoutedEventArgs e)
        {
            zoom_changed(sender, 1);
        }

        //--- ShowBoxBorders_Click --------------------------------------------
        private void ShowBoxBorders_Click(object sender, RoutedEventArgs e)
        {
            ShowBoxBorders.IsChecked = _LayoutDef.ShowBoxBorders = !_LayoutDef.ShowBoxBorders;
            e.Handled = true;
            _ActiveZoom.Focus();
        }

        //--- LabelFrame_ScrollChanged ---------------------------------------------
        private void LabelFrame_ScrollChanged(object sender, ScrollChangedEventArgs e)
        {
            draw_ruler();
        }

        //--- show_layers --------------------------------------------------------------
        //DID MOVE
        private void show_layers(object sender, RoutedEventArgs e)
        {
            foreach(Image img  in _LabelImg)
            {
                _LayoutDef.LabelVisible = (this.LabelPropertiesVM.IsLabelControlChecked==true) || 
                                          (this.LabelPropertiesVM.IsColorControlChecked==true);
                if (this.LabelPropertiesVM.IsLabelControlChecked==true) img.Visibility=Visibility.Visible;
                else                                             img.Visibility=Visibility.Hidden;
            }

            if (_LayoutDef.ColorLayer==null) this.LabelPropertiesVM.IsColorControlChecked=false;
            if (this.LabelPropertiesVM.IsVariableControlChecked==true)
            {
                if (this.LabelPropertiesVM.IsColorControlChecked==true)
                {   
                    foreach(Image img in _VarDataImg) img.Visibility  = Visibility.Hidden;
                    for(int i=0; i<_ColorImg.Count(); i++)
                    {
                        if (_LayoutDef.LabelBitmap.Count>i) _ColorImg[i].OpacityMask = new ImageBrush(_LayoutDef.LabelBitmap[i]);
                        _ColorImg[i].Visibility = Visibility.Visible;
                    }
                }
                else
                { 
                    foreach(Image img in _ColorImg)   img.Visibility  = Visibility.Hidden;
                    foreach(Image img in _VarDataImg) img.Visibility  = Visibility.Visible;
                }
            }
            else
            {
                if (this.LabelPropertiesVM.IsColorControlChecked==true)
                {
                    foreach(Image img in _VarDataImg) img.Visibility  = Visibility.Hidden;
                    foreach(Image img in _ColorImg)   img.OpacityMask = null;
                }
                else
                {
                    this.LabelPropertiesVM.IsVariableControlChecked  = true;
                    foreach(Image img in _ColorImg)   img.Visibility = Visibility.Hidden;
                    foreach(Image img in _VarDataImg) img.Visibility = Visibility.Visible;
                }
            }
            _LayoutDef.ColoredLabel = (this.LabelPropertiesVM.IsColorControlChecked==true) && 
                                      (this.LabelPropertiesVM.IsVariableControlChecked==true);
            /*
            foreach(Image img  in _LabelImg)
            {
                _LayoutDef.LabelVisible = (LabelVisible.IsChecked==true || ColorVisible.IsChecked==true);
                if (LabelVisible.IsChecked==true) img.Visibility=Visibility.Visible;
                else                              img.Visibility=Visibility.Hidden;
            }

            if (_LayoutDef.ColorLayer==null) ColorVisible.IsChecked=false;
            if (VariableVisible.IsChecked==true)
            {
                if (ColorVisible.IsChecked==true)
                {   
                    foreach(Image img in _VarDataImg) img.Visibility  = Visibility.Hidden;
                    for(int i=0; i<_ColorImg.Count(); i++)
                    {
                        if (_LayoutDef.LabelBitmap.Count>i) _ColorImg[i].OpacityMask = new ImageBrush(_LayoutDef.LabelBitmap[i]);
                        _ColorImg[i].Visibility = Visibility.Visible;
                    }
                }
                else
                { 
                    foreach(Image img in _ColorImg)   img.Visibility  = Visibility.Hidden;
                    foreach(Image img in _VarDataImg) img.Visibility  = Visibility.Visible;
                }
            }
            else
            {
                if (ColorVisible.IsChecked==true)
                {
                    foreach(Image img in _VarDataImg) img.Visibility  = Visibility.Hidden;
                    foreach(Image img in _ColorImg)   img.OpacityMask = null;
                }
                else
                {
                    VariableVisible.IsChecked   = true;
                    foreach(Image img in _ColorImg)   img.Visibility = Visibility.Hidden;
                    foreach(Image img in _VarDataImg) img.Visibility = Visibility.Visible;
                }
            }
            _LayoutDef.ColoredLabel = (ColorVisible.IsChecked==true) && (VariableVisible.IsChecked==true);
            */
        }

        //--- labelSizeChanged ---------------------------------------
        private void labelSizeChanged()
        {
            draw_ruler();
        }

     }
}
