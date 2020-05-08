using System.Windows;
using System.Windows.Controls;

namespace RX_LabelComposer.External
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
    }
}
