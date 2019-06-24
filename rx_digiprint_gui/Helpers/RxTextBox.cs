using RX_Common;
using System;
using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Resources;
using System.Windows.Threading;

namespace RX_DigiPrint.Helpers
{
    public class RxTextBox : TextBox
    {
        //--- Constructor ----------------------------
        public RxTextBox ()
        {
            Style = Application.Current.Resources["RxTextBox"] as Style;
            PreviewTouchUp += RxTextBox_TouchUp;
         //   TouchUp += RxTextBox_TouchUp;
            PreviewMouseUp += RxTextBox_MouseUp;
        }

        //--- OpenPad -----------------------------
        public void OpenPad()
        {
            RxTextBox_TouchUp(null, null);
        }

        //--- RxTextBox_MouseUp --------------------------------------
        void RxTextBox_MouseUp(object sender, MouseButtonEventArgs e)
        {
            if (!IsPadActive) RxTextBox_TouchUp(sender, null);
        }

        //--- Property IsPadActive ------------------------------------------------
        public bool IsPadActive
        {
            get { return (bool)GetValue(IsPadActiveProperty); }
            set { SetValue(IsPadActiveProperty, value); }
        }

        // Using a DependencyProperty as the backing store for IsPadActive.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty IsPadActiveProperty =
            DependencyProperty.Register("IsPadActive", typeof(bool), typeof(RxTextBox), new PropertyMetadata(false));

        //--- RxTextBox_TouchUp -----------------------------------
        public void RxTextBox_TouchUp(object sender, TouchEventArgs e)
        {
            IsPadActive = true;
            RxTextPad pad = new RxTextPad(this);
            bool? result=pad.ShowDialog();
            IsPadActive = false;
            if (result!=null && (bool)result == true)
            {
                Text = pad.Result;
                RaiseEvent(new RoutedEventArgs(LostFocusEvent, this));
            }
        }

        //--- OnKeyDown ------------------------------------
        private void OnKeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            if (e.Key.ToString() == "Return")
                RaiseEvent(new RoutedEventArgs(FocusManager.LostFocusEvent));   // force refresh!            
            else
            {
                double d = RxKeys.Value(e);            
                if (d!=0)
                {
                    try
                    { 
                        double val  = double.Parse(Text, CultureInfo.InvariantCulture) +d;
                        Text = val.ToString(CultureInfo.InvariantCulture);
                        RaiseEvent(new RoutedEventArgs(FocusManager.LostFocusEvent));   // force refresh!
                    }
                    catch (Exception)
                    {
                        return;
                    }
                }
            }
        }

    }
}
