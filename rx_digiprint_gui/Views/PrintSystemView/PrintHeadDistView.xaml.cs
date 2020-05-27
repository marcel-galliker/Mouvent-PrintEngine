﻿using RX_Common;
using RX_DigiPrint.Converters;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Diagnostics;
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

namespace RX_DigiPrint.Views.PrintSystemView
{
    /// <summary>
    /// Interaction logic for PrintHeadDistView.xaml
    /// </summary>
    public partial class PrintHeadDistView : UserControl
    {
        private int            _HeadsPerColor=1;
        private bool           _Reverse=false;
        private bool           _AdjustmentSupported=false;
        private const double   _StrokeDist = 25.4 / 1200;
            
        static ControlTemplate _ButtonTemplate;

        //--- Constructor -----------------------------------
        public PrintHeadDistView(int headNo, int headsPerColor)
        {
            InitializeComponent();
        
            HeadNo=headNo;
            _HeadsPerColor = headsPerColor;
            _PrinterType_Changed();
            _CreateHeads();
            RxGlobals.PrintSystem.PropertyChanged += _PrintSystem_PropertyChanged;
            for (int i=0; i<RxGlobals.InkSupply.List.Count; i++)
            {
                RxGlobals.InkSupply.List[i].PropertyChanged += _PrintSystem_PropertyChanged;            
            }
        }

        //--- Property MyProperty ---------------------------------------
        private int _ClusterNo=-1;
        public int HeadNo   
        {
            get { return _ClusterNo; }
            set 
            { 
                _ClusterNo = value; 
                Title.Text = String.Format("Cluster {0}", _ClusterNo+1);
                _update();
            }
        }
        
        //--- _PrinterType_Changed ---------------------------------------------
        private void _PrinterType_Changed()
        {
            switch (RxGlobals.PrintSystem.PrinterType)
            {
            case EPrinterType.printer_cleaf:    _AdjustmentSupported = true;  break;
            case EPrinterType.printer_LB701:    _AdjustmentSupported = true;  break;
            case EPrinterType.printer_LB702_UV: _AdjustmentSupported = true;  break;
            case EPrinterType.printer_LB702_WB: _AdjustmentSupported = true;  break;
            case EPrinterType.printer_LH702:    _AdjustmentSupported = true;  break;
            case EPrinterType.printer_DP803:    _AdjustmentSupported = true;  break;
            case EPrinterType.printer_TX801:    _AdjustmentSupported = true;  _Reverse=true; break;
            case EPrinterType.printer_TX802:    _AdjustmentSupported = true;  _Reverse=true; break;
            default:                            _AdjustmentSupported = false; break;
            }            
        }

        //--- _PrintSystem_PropertyChanged ------------------------------------------
        void _PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("PrinterType")) _PrinterType_Changed();
            _update();
        }

        //--- _CreateHeads -----------------------------------------
        private void _CreateHeads()
        {
            int i;
            int colorNo;
            int distNo=_ClusterNo*TcpIp.HEAD_CNT;
            double x=0, x1, y=0;
            double width=50;
            double height=50;
            double gap=10;
            double top=40;

            colorNo = (_ClusterNo*TcpIp.HEAD_CNT) / RxGlobals.PrintSystem.HeadCnt; 
            for (i=0; i<TcpIp.HEAD_CNT; i++, distNo++)
            {
                x = (width+gap)*i+gap/2;
                
                Button but = new Button()
                {
                    BorderThickness = new Thickness(1),
                    Margin = new Thickness(0, -1, 0, 0),                         
                    Width=width, 
                    Height=height,
                    Tag=i,
                    Content = new TextBlock()
                    {   
                        Text="?-"+(distNo+1).ToString(), 
                        HorizontalAlignment=HorizontalAlignment.Center, 
                        VerticalAlignment=VerticalAlignment.Center,
                    }                  
                };
                if (_ButtonTemplate==null) _ButtonTemplate=but.Template;
                if (!_AdjustmentSupported) but.Template =  Application.Current.Resources["ButtonNoMouseOver"] as ControlTemplate;
                but.Click += Button_Clicked;
                Canvas.SetTop(but,  top+gap/2);
                Canvas.SetLeft(but, x);
                Canvas.Children.Add(but);                

                if (_HeadsPerColor!=0)
                {
                    if (RxGlobals.PrintSystem.IsScanning)
                    {
                        if (distNo%_HeadsPerColor==0)
                        {
                            //--- offset arrow --------------
                            y  = 10;
                            x1 = x+width;
                            Canvas.Children.Add(new Line(){Stroke=Brushes.Black, X1=x,     X2=x,  Y1=y-5, Y2=y+5});
                            Canvas.Children.Add(new Line(){Stroke=Brushes.Black, X1=x,     X2=x1, Y1=y,   Y2=y});
                            Canvas.Children.Add(new Line(){Stroke=Brushes.Black, X1=x1-3,  X2=x1, Y1=y-3, Y2=y});
                            Canvas.Children.Add(new Line(){Stroke=Brushes.Black, X1=x1-3,  X2=x1, Y1=y+3, Y2=y});

                            //--- offset text box -----------
                            RxNumBox offset = new RxNumBox(){Width=width, HorizontalContentAlignment=HorizontalAlignment.Right};
                            offset.Tag = distNo;//colorNo++;
                            offset.FontSize = (double)Application.Current.Resources["FontSizeSmall"];
                            offset.LostFocus += _text_LostFocus;
                            Canvas.SetLeft(offset, x);
                            Canvas.SetTop(offset,  16);
                            Canvas.Children.Add(offset);
                        }
                    }
                    else
                    {
                        if (distNo%_HeadsPerColor==_HeadsPerColor-1)
                        {
                            //--- offset arrow --------------
                            y  = 10;
                            x1 = x+width;
                            Canvas.Children.Add(new Line(){Stroke=Brushes.Black, X1=x1,   X2=x1,  Y1=y-5, Y2=y+5});
                            Canvas.Children.Add(new Line(){Stroke=Brushes.Black, X1=x1,   X2=x,   Y1=y,   Y2=y});
                            Canvas.Children.Add(new Line(){Stroke=Brushes.Black, X1=x+3,  X2=x,   Y1=y-3, Y2=y});
                            Canvas.Children.Add(new Line(){Stroke=Brushes.Black, X1=x+3,  X2=x,   Y1=y+3, Y2=y});

                            TextBlock text = new TextBlock(){Text="mm", FontSize=(double)Application.Current.Resources["FontSizeSmall"], Background=Brushes.White};
                            Canvas.SetLeft(text, x+width/2);
                            Canvas.SetTop (text, y-8);
                            Canvas.Children.Add(text);

                            //--- offset text box -----------
                            RxNumBox offset = new RxNumBox(){Width=width, HorizontalContentAlignment=HorizontalAlignment.Right};
                            offset.Tag = colorNo++;
                            offset.FontSize = (double)Application.Current.Resources["FontSizeSmall"];
                            offset.LostFocus += _text_LostFocus;
                            Canvas.SetLeft(offset, x);
                            Canvas.SetTop(offset,  16);
                            Canvas.Children.Add(offset);
                        }
                        if (distNo%_HeadsPerColor==_HeadsPerColor-2)
                        {
                            //--- offset arrow --------------
                            y  = 10;
                            x1 = x+width;
                            Canvas.Children.Add(new Line(){Stroke=Brushes.Black, X1=x1,   X2=x1,  Y1=y-5, Y2=y+5});
                            Canvas.Children.Add(new Line(){Stroke=Brushes.Black, X1=x1,   X2=x,   Y1=y,   Y2=y});
                            Canvas.Children.Add(new Line(){Stroke=Brushes.Black, X1=x+3,  X2=x,   Y1=y-3, Y2=y});
                            Canvas.Children.Add(new Line(){Stroke=Brushes.Black, X1=x+3,  X2=x,   Y1=y+3, Y2=y});

                            TextBlock text = new TextBlock(){Text="dt", FontSize=(double)Application.Current.Resources["FontSizeSmall"], Background=Brushes.White};
                            Canvas.SetLeft(text, x+width/2);
                            Canvas.SetTop (text, y-8);
                            Canvas.Children.Add(text);

                            //--- offset text box -----------
                            RxNumBox offset = new RxNumBox(){Width=width, HorizontalContentAlignment=HorizontalAlignment.Right};
                            offset.Tag = 100+colorNo;
                            offset.FontSize = (double)Application.Current.Resources["FontSizeSmall"];
                            offset.LostFocus += _text_LostFocus;
                            Canvas.SetLeft(offset, x);
                            Canvas.SetTop(offset,  16);
                            Canvas.Children.Add(offset);
                        }
                    }
                }

                //--- distance arrow -----------------
                x1 = x+width/2;
                y  = height+gap/2+top;
                Canvas.Children.Add(new Line(){Stroke=Brushes.Black, X1=x1,   X2=x1, Y1=y, Y2=y+80});
                Canvas.Children.Add(new Line(){Stroke=Brushes.Black, X1=x1-5, X2=x1, Y1=y+5, Y2=y});
                Canvas.Children.Add(new Line(){Stroke=Brushes.Black, X1=x1+5, X2=x1, Y1=y+5, Y2=y});

                //--- distance text box fore ------------------
                RxNumBox dist = new RxNumBox(){Width=width, HorizontalContentAlignment=HorizontalAlignment.Right};
                dist.Tag = 200+distNo;
                dist.FontSize = (double)Application.Current.Resources["FontSizeSmall"];
                dist.LostFocus += _text_LostFocus;
                Canvas.SetLeft(dist, x);
                Canvas.SetTop(dist, y+12);
                Canvas.Children.Add(dist);

                //--- distance text box back ------------------
                if (true)
                {
                    RxNumBox distback = new RxNumBox(){Width=width, HorizontalContentAlignment=HorizontalAlignment.Right};
                    distback.Tag = 300+distNo;
                    distback.FontSize = (double)Application.Current.Resources["FontSizeSmall"];
                    distback.LostFocus += _text_LostFocus;
                    Canvas.SetLeft(distback, x);
                    Canvas.SetTop(distback, y+35);
                    Canvas.Children.Add(distback);
                }

            }
            this.Width = TcpIp.HEAD_CNT*(width+gap);
            Canvas.Children.Add(new Line(){Stroke=Brushes.Black, X1=0, X2=TcpIp.HEAD_CNT*(width+gap), Y1=y+80, Y2=y+80});

            _update();
        }

        //--- _tag2head --------------------------------------
        private void _tag2head(int tag, out int isNo, out int headNo)
        {
            isNo=0;
            headNo=0;
            if (RxGlobals.PrintSystem.HeadCnt!=0)
            {
                int no = tag%100;
                if (RxGlobals.PrintSystem.IsTx)
                {
                    isNo   = RxGlobals.PrintSystem.IS_Order[no/RxGlobals.PrintSystem.HeadCnt];
                    if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_TX802)
                        headNo = RxGlobals.PrintSystem.HeadCnt*isNo + RxGlobals.PrintSystem.HeadCnt-1-(no%RxGlobals.PrintSystem.HeadCnt);
                    else
                        headNo = RxGlobals.PrintSystem.HeadCnt*isNo + no%RxGlobals.PrintSystem.HeadCnt;
                }
                else if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_test_table)
                {
                    isNo   = no;
                    headNo = 0;
                }
                else
                {
                    isNo   = RxGlobals.PrintSystem.IS_Order[(_ClusterNo*TcpIp.HEAD_CNT)/RxGlobals.PrintSystem.HeadCnt];
                    headNo = RxGlobals.PrintSystem.HeadCnt*isNo + no%RxGlobals.PrintSystem.HeadCnt;
                }
            }            
        }

        //--- _text_LostFocus ---------------------------------------------------
        void _text_LostFocus(object sender, RoutedEventArgs e)
        {
            RxNumBox text = sender as RxNumBox;    
            if (text!=null && RxGlobals.PrintSystem.HeadCnt!=0)
            {
                try
                {
                    int tag= (int)text.Tag;
                    int inkSupply, headNo;
                    _tag2head(tag, out inkSupply, out headNo);
                    switch(tag/100)
                    {
                        case 0:     _tag2head(tag, out inkSupply, out headNo);
                                    if (RxGlobals.PrintSystem.ColorOffset[inkSupply]!=text.Value)
                                    {
                                        RxGlobals.PrintSystem.ColorOffset[inkSupply] = text.Value;
                                        RxGlobals.PrintSystem.Changed = true;
                                        _update();
                                    }
                                    break;

                        case 1:     if (RxGlobals.PrintSystem.ColorOffset[inkSupply]!=text.Value)
                                    {
                                        RxGlobals.PrintSystem.ColorOffset[inkSupply] = Math.Round(text.Value*_StrokeDist, 3);
                                        RxGlobals.PrintSystem.Changed = true;
                                        _update();
                                    }
                                    break;

                        case 2:     if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_test_table) headNo=inkSupply;
                                    if (RxGlobals.PrintSystem.HeadDist[headNo] != text.Value)
                                    {
                                        if (RxGlobals.PrintSystem.IsScanning)
                                        { 
                                            RxGlobals.PrintSystem.HeadDist[headNo] = text.Value;
                                        }
                                        else
                                        { 
                                            double diff=text.Value-RxGlobals.PrintSystem.HeadDist[headNo];
                                            do
                                            {
                                                RxGlobals.PrintSystem.HeadDist[headNo++] += diff;
                                            }
                                            while ((headNo%RxGlobals.PrintSystem.HeadCnt)!=0);
                                            RxGlobals.PrintSystem.OnPropertyChanged();  // call _update() for all heads
                                        }
                                        RxGlobals.PrintSystem.Changed = true;
                                    }
                                    break;
                        
                        case 3:     if(RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_test_table) headNo=inkSupply;
                                    if (RxGlobals.PrintSystem.HeadDistBack[headNo] != text.Value)
                                    {
                                        RxGlobals.PrintSystem.HeadDistBack[headNo] = text.Value;
                                        RxGlobals.PrintSystem.Changed = true;
                                    }
                                    break;
                    }
                }
                catch(Exception)
                {
                }
            }        
        }

        //--- Button_Clicked ---------------------------
        private void Button_Clicked(object sender, RoutedEventArgs e)
        { 
            if (_AdjustmentSupported)
            {
                Int32 tag=_ClusterNo*TcpIp.HEAD_CNT+Convert.ToInt32((sender as Button).Tag);
                int inkSupply= RxGlobals.PrintSystem.IS_Order[tag/(int)RxGlobals.PrintSystem.HeadCnt];
                int no       =tag%(int)RxGlobals.PrintSystem.HeadCnt +1;
                
                HeadAdjustment dlg = new HeadAdjustment(sender as Control, inkSupply, no);
                dlg.ShowDialog();
            }
        }

        //--- _update -----------------------------------------------------------------------
        private void _update()
        {
            if (_ClusterNo<0) return;
            Visibility visible = Visibility.Visible;

            switch (RxGlobals.PrintSystem.PrinterType)
            {
                case EPrinterType.printer_test_table:   visible=Visibility.Visible;     Border.Height=160; _Reverse=false;
                                                        break;
                case EPrinterType.printer_TX801:
                case EPrinterType.printer_TX802:        visible=Visibility.Visible;     Border.Height=160; _Reverse=true;
                                                        break;
                default:                                visible=Visibility.Collapsed;   Border.Height=140; _Reverse=false; break;
            }

            DirText.Visibility = visible;
            int cnt=(int)(RxGlobals.PrintSystem.ColorCnt*RxGlobals.PrintSystem.HeadCnt);
            if (_Reverse) Title.Text = String.Format("Cluster {0}", (cnt+TcpIp.HEAD_CNT-1)/TcpIp.HEAD_CNT-_ClusterNo);
            else          Title.Text = String.Format("Cluster {0}", 1+_ClusterNo);
            foreach (var ctrl in Canvas.Children)
            {
                RxNumBox text = ctrl as RxNumBox;
                if (text!=null && RxGlobals.PrintSystem.ColorCnt!=0)
                {
                    int tag = Convert.ToInt32(text.Tag);
                    int inkSupply, headNo;
                    _tag2head(tag, out inkSupply, out headNo);
                    if (!text.IsFocused)
                    {
                        try
                        {
                            switch(tag/100)
                            {
                                case 0: _tag2head(tag, out inkSupply, out headNo);
                                        text.Text = RxGlobals.PrintSystem.ColorOffset[inkSupply].ToString();break;
                                case 1: text.Text = Math.Round(RxGlobals.PrintSystem.ColorOffset[inkSupply]/_StrokeDist).ToString();break;
                                case 2: if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_test_table)
                                            text.Text = RxGlobals.PrintSystem.HeadDist[inkSupply].ToString();
                                        else 
                                            text.Text = RxGlobals.PrintSystem.HeadDist[headNo].ToString(); 
                                        break;
                                case 3: text.Visibility = visible;                          
                                        if(RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_test_table)
                                            text.Text = RxGlobals.PrintSystem.HeadDistBack[inkSupply].ToString();
                                        else
                                            text.Text = RxGlobals.PrintSystem.HeadDistBack[headNo].ToString(); 
                                        break;
                            }
                        }
                        catch(Exception)
                        { }
                    }
                }
                else
                {
                    Button button = ctrl as Button;
                    if (button==null) continue;
                    TextBlock name = button.Content as TextBlock;
                    if (name!=null)
                    {
                        try 
                        {
                            int tag = Convert.ToInt32(button.Tag);
                            int inkSupply, headNo;
                            _tag2head(_ClusterNo*TcpIp.HEAD_CNT+tag, out inkSupply, out headNo);
                            int no=1+headNo%RxGlobals.PrintSystem.HeadCnt;
                            if (RxGlobals.InkSupply.List[inkSupply].InkType==null) button.Background = Brushes.WhiteSmoke;
                            else
                            {
                                if (_AdjustmentSupported) button.Template = _ButtonTemplate;
                                else                      button.Template =  Application.Current.Resources["ButtonNoMouseOver"] as ControlTemplate;

                                button.Background = new SolidColorBrush(RxGlobals.InkSupply.List[inkSupply].InkType.Color);
                                button.Foreground = new SolidColorBrush(RxGlobals.InkSupply.List[inkSupply].InkType.ColorFG);
                                name.Text = new ColorCode_Str().Convert(RxGlobals.InkSupply.List[inkSupply].InkType.ColorCode, null, inkSupply, null).ToString()+"-"+no.ToString();
                            }
                        }
                        catch(Exception)
                        {                    
                            name.Background = Brushes.WhiteSmoke;
                        }
                    }
                }
            }
        }
    }
}
