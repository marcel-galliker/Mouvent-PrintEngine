using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Helpers
{
    class MvtInkSupplyRadioButton : RadioButton
    {
        public MvtInkSupplyRadioButton()
            : base()
        {
            ShowStatusBorder = true;
        }
        
        public static readonly DependencyProperty ShowStatusBorderProperty =
            DependencyProperty.RegisterAttached("ShowStatusBorder", typeof(bool), typeof(MvtInkSupplyRadioButton),
            new PropertyMetadata(true, new PropertyChangedCallback(OnShowStatusBorderPropertyChanged)));

        public bool ShowStatusBorder 
        {
            get
            {
                return (bool)GetValue(ShowStatusBorderProperty);
            }
            set
            {
                SetValue(ShowStatusBorderProperty, value);
            }
        }

        public static void SetShowStatusBorder(UIElement element, bool value)
        {
            element.SetValue(ShowStatusBorderProperty, value);
        }

        public static bool GetShowStatusBorder(UIElement element)
        {
            return (bool)element.GetValue(ShowStatusBorderProperty);
        }

        private static void OnShowStatusBorderPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {

        }
    }
}
