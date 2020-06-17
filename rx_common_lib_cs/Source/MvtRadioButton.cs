using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;

namespace RX_Common
{
    public class MvtRadioButton: RadioButton
    {
        //--- constructor  -------------------------------------
        public MvtRadioButton()
        {
            Error   = false; // unused?
            Warning = false; // unused?
            Last = false;
            PreviewMouseDown += MvtRadioButton_PreviewMouseDown;
        }

        //--- MvtRadioButton_PreviewMouseDown ------------------------
        void MvtRadioButton_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            e.Handled=true;
            this.IsChecked = true;
        }

        //--- property Error -----------------------------------------
        public bool Error
        {
            get { return (bool)GetValue(ErrorProperty); }
            set { SetValue(ErrorProperty, value); }
        }
        public static readonly DependencyProperty ErrorProperty = DependencyProperty.Register("Error", typeof(bool), typeof(MvtRadioButton));          

        //--- property Warning -----------------------------------------
        public bool Warning
        {
            get { return (bool)GetValue(WarningProperty); }
            set { SetValue(WarningProperty, value); }
        }
        public static readonly DependencyProperty WarningProperty = DependencyProperty.Register("Warning", typeof(bool), typeof(MvtRadioButton));          

        //--- property Last -----------------------------------------
        public bool Last
        {
            get { return (bool)GetValue(LastProperty); }
            set { SetValue(LastProperty, value); }
        }
        public static readonly DependencyProperty LastProperty = DependencyProperty.Register("Last", typeof(bool), typeof(MvtRadioButton)); 
            
    }
}
