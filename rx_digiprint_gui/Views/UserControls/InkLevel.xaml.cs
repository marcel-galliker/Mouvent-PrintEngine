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
        
        //--- InkLevel_DataContextChanged ----------------------------------------------------------------
        private void InkLevel_DataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            _InkSupply = DataContext as InkSupply;
            if (_InkSupply!=null)
            {
                _InkSupply.PropertyChanged += _InkSupply_PropertyChanged;
                _update_InkType();
                _update_level();
            }
        }

        //--- _InkSupply_PropertyChanged ------------------------------
        void _InkSupply_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("InkType")) _update_InkType();
            else if (e.PropertyName.Equals("CanisterLevel")) _update_level();
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
            if (_InkSupply==null) return;

            Progress.Value = (UInt32)_InkSupply.CanisterLevel;
            if (_InkSupply.CanisterLevel==TcpIp.INVALID_VALUE)
            { 
                Progress.Value    = 0;
                Progress.ValueStr = "--- Kg";
            }
            else
            { 
                Progress.Value    = (uint)(100.0*_InkSupply.CanisterLevel/25000.0);
                Progress.ValueStr = string.Format("{0}.{1} Kg", _InkSupply.CanisterLevel/1000, Math.Abs(_InkSupply.CanisterLevel%1000)/100);
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
