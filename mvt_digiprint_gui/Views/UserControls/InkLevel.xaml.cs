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

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for InkLevel.xaml
    /// </summary>
    public partial class InkLevel : UserControl
    {
        private InkSupply _InkSupply;

        public InkLevel()
        {
            InitializeComponent();
            Progress.Visibility = Visibility.Collapsed;
        }

        public InkLevel(InkSupply InputInkSupply)
        {
            InitializeComponent();
            Progress.Visibility = Visibility.Collapsed;
            _InkSupply = InputInkSupply;
            if (_InkSupply != null)
            {
                _InkSupply.PropertyChanged += _InkSupply_PropertyChanged;
                _update_InkType();
                _update_level();
            }
        }

        //--- Property Enabled ---------------------------------------
        private bool _Enabled = true;
        public bool Enabled
        {
            get { return _Enabled; }
            set 
            { 
                _Enabled = value;
                Progress.ShowText = _Enabled;
            }
        }

        //--- _InkSupply_PropertyChanged ------------------------------
        void _InkSupply_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("InkType")) _update_InkType();
            else if (e.PropertyName.Equals("CanisterLevel") || e.PropertyName.Equals("CanisterErr")) _update_level();
        }

        //--- _update_InkType --------------------------
        private void _update_InkType()
        {
            InkType ink = _InkSupply.InkType;
            if (ink!=null)
            {
                Progress.Visibility = Visibility.Visible;
                Progress.Text       = ink.Name;
                Progress.TextColor  = new SolidColorBrush(ink.ColorFG);
                Progress.Color      = new SolidColorBrush(ink.Color);
                _update_level();
            }
            else Progress.Visibility = Visibility.Collapsed;
        }

        //--- _update_level ------------------------------
        private void _update_level()
        {
            double max;
            if (_InkSupply==null) return;
            if (RxGlobals.PrintSystem.IsTx) max = 10000; 
            else max=20000;

            Progress.Value = (UInt32)_InkSupply.CanisterLevel;
            if (_InkSupply.CanisterLevel==TcpIp.INVALID_VALUE)
            { 
                Progress.Value    = 0;
                Progress.ValueStr = "--- Kg";
            }
            else
            { 
                Progress.Value    = (uint)(100.0*_InkSupply.CanisterLevel/max);
                Progress.ValueStr = string.Format("{0}.{1} Kg", _InkSupply.CanisterLevel/1000, Math.Abs(_InkSupply.CanisterLevel%1000)/100);
            }
            switch(_InkSupply.CanisterErr)
            {
                case ELogType.eErrWarn: Progress.Background = new SolidColorBrush(Colors.Yellow);       break;
                case ELogType.eErrCont: Progress.Background = new SolidColorBrush(Colors.Red);          break;
                default:                Progress.Background = new SolidColorBrush(Colors.Transparent);  break;
            }
        }

        //--- UserControl_PreviewMouseDown --------------------------------
        private void UserControl_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            if(_Enabled)
            {
                InkInfoWnd wnd = new InkInfoWnd(_InkSupply);
                wnd.ShowDialog();
            }
            e.Handled = true;
        }
    }
}
