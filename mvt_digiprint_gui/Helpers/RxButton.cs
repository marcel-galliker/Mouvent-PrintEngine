using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Helpers
{
    public class RxButton : Button
    {
        public RxButton()
        {
            IsChecked = false;
        }

        //--- Property IsChecked ---------------------
        public bool IsChecked   
        {
            get { return (bool)GetValue(IsButtonCheckedProperty); }
            set { SetValue(IsButtonCheckedProperty, value); }
        }

        // Using a DependencyProperty as the backing store for IsButtonChecked.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty IsButtonCheckedProperty =
            DependencyProperty.Register("IsChecked", typeof(bool), typeof(RxButton));               

        //--- Property IsBusy ---------------------
        public bool IsBusy   
        {
            get { return (bool)GetValue(IsButtonBusyProperty); }
            set { SetValue(IsButtonBusyProperty, value); }
        }

        // Using a DependencyProperty as the backing store for IsButtonChecked.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty IsButtonBusyProperty =
            DependencyProperty.Register("IsBusy", typeof(bool), typeof(RxButton));               
    }
}
