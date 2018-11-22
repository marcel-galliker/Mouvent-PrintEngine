using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using RX_DigiPrint.Services;
using RX_Common;

namespace RX_DigiPrint.Views.UserControls
{
    public partial class ConsumablesView : UserControl
    {
        private List<RxProgress> _InkProgress = new List<RxProgress>();
        private RxProgress       _ActItem = null;

        public ConsumablesView()
        {
            InitializeComponent();
            DataContext = this;

            for (int i=0; i<TcpIp.InkSupplyCnt+2; i++)
            {
                 _InkProgress.Add(new RxProgress());
                 _InkProgress[i].Visibility = Visibility.Collapsed;
                 _InkProgress[i].PreviewMouseDown += ConsumablesView_PreviewMouseDown;
                 Grid.SetColumn(_InkProgress[i], i);
                 MainGrid.Children.Add(_InkProgress[i]);
            }
            RxGlobals.PrintSystem.PropertyChanged   += PrintSystem_PropertyChanged;
        }

        private void ConsumablesView_PreviewMouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            int i;
            e.Handled = true;

            if (RxGlobals.User.UserType<EUserType.usr_service) return;

            for (i=0; i<MainGrid.Children.Count; i++)
            {
                if (MainGrid.Children[i]==sender)
                {
                    Point pos = this.PointToScreen(new Point(0, ActualHeight));
                    ScaleCalibration dlg = new ScaleCalibration(sender as Control, pos, i);
                    dlg.ShowDialog();
                    break;
                }
            }
        }

        //--- Property CalbrationView ---------------------------------------
        private ScaleCalibration _CalibrationView=null;
        public ScaleCalibration CalibrationView
        {
            get { return _CalibrationView; }
            set 
            { 
                _CalibrationView = value;
                if (_CalibrationView != null) _CalibrationView.IsVisibleChanged += _CalibrationView_IsVisibleChanged;
            }
        }

        //--- UserControl_IsVisibleChanged ----------------------------------
        private void UserControl_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
           if ((bool)e.NewValue) RxGlobals.Timer.TimerFct += Tick;
           else                  RxGlobals.Timer.TimerFct -= Tick;
        }

        //--- _CalibrationView_IsVisibleChanged ----------------------------
        void _CalibrationView_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue)
            {
                _ActItem.Background = App.Current.Resources["RX.MouseOver.Background"] as Brush;
            }
            else
            {
                if (_ActItem!=null) _ActItem.Background = Brushes.Transparent;
                _ActItem = null;
            }
        }
       
        //--- set_canister_level --------------------------
        public void set_canister_level(int no, int value)
        {
            if (value==TcpIp.INVALID_VALUE)
            { 
                _InkProgress[no].Value    = 0;
                _InkProgress[no].ValueStr = "--- Kg";
            }
            else
            { 
                _InkProgress[no].Value    = (uint)(100.0*value/25000.0);
                _InkProgress[no].ValueStr = string.Format("{0}.{1} Kg", value/1000, (value%1000)/100);
            }
        }

        //--- Tick ------------------------------------
        private void Tick(int tick)
        {
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_FLUID_STAT);

            for (int i=0; i<RxGlobals.PrintSystem.ColorCnt+3; i++)
            {
                set_canister_level(i, RxGlobals.Scales[i].weight);
            }
        }
        
        //--- PrintSystem_PropertyChanged ---------------------------------------------------
        void PrintSystem_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            int i;
            int no;

            try {
                for (i=0; i<RxGlobals.PrintSystem.ColorCnt+2; i++)
                {
                    _InkProgress[i].Visibility = Visibility.Visible;
                    if (i==0)
                    {
                        _InkProgress[i].Text       = "Flush";
                        _InkProgress[i].TextColor  = Brushes.Black;
                        _InkProgress[i].Color      = new SolidColorBrush(Color.FromRgb(0xf0,0xf0,0xf0));
                    }
                    else if (i==RxGlobals.PrintSystem.ColorCnt+1)
                    {
                        _InkProgress[i].Text       = "Waste";
                        _InkProgress[i].TextColor  = Brushes.White;
                        _InkProgress[i].Color      = new SolidColorBrush(Color.FromRgb(0x80,0x80,0x80));
                    }
                    else if (RxGlobals.InkSupply.List[i-1].InkType!=null)
                    {
                        if (RxGlobals.PrintSystem.Reverse) no = RxGlobals.PrintSystem.ColorCnt-i;
                        else          no = i-1;
                        if (RxGlobals.InkSupply.List[no].InkType!=null)
                        {
                            _InkProgress[i].Text       = RxGlobals.InkSupply.List[no].InkType.Name;
                            _InkProgress[i].TextColor  = new SolidColorBrush(RxGlobals.InkSupply.List[no].InkType.ColorFG);
                            _InkProgress[i].Color      = new SolidColorBrush(RxGlobals.InkSupply.List[no].InkType.Color);
                        }
                    }
                }
                while (i<_InkProgress.Count()) _InkProgress[i++].Visibility = Visibility.Collapsed;
            }
            catch(Exception)
            { }
        }

    }
}
