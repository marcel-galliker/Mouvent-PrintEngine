using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;

namespace RX_Common
{
    public class RxRadioButton: RadioButton
    {
        //--- constructor  -------------------------------------
        public RxRadioButton()
        {
            Error   = false;
            Warning = false;
            Last    = false;
            PreviewMouseDown += RxRadioButton_PreviewMouseDown;
        }

        //--- RxRadioButton_PreviewMouseDown ------------------------
        void RxRadioButton_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            e.Handled=true;
            this.IsChecked = (bool)!this.IsChecked;
        }

        //--- property Error -----------------------------------------
        public bool Error
        {
            get { return (bool)GetValue(ErrorProperty); }
            set { SetValue(ErrorProperty, value); }
        }
        public static readonly DependencyProperty ErrorProperty = DependencyProperty.Register("Error", typeof(bool), typeof(RxRadioButton));          

        //--- property Warning -----------------------------------------
        public bool Warning
        {
            get { return (bool)GetValue(WarningProperty); }
            set { SetValue(WarningProperty, value); }
        }
        public static readonly DependencyProperty WarningProperty = DependencyProperty.Register("Warning", typeof(bool), typeof(RxRadioButton));          

        //--- property Last -----------------------------------------
        public bool Last
        {
            get { return (bool)GetValue(LastProperty); }
            set { SetValue(LastProperty, value); }
        }
        public static readonly DependencyProperty LastProperty = DependencyProperty.Register("Last", typeof(bool), typeof(RxRadioButton)); 
            
    }
}
