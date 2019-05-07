using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Helpers
{
    public class RxRadioButton: RadioButton
    {
        //--- constructor  -------------------------------------
        public RxRadioButton()
        {
            Error   = false;
            Warning = false;
            Last    = false;
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
