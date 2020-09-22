using RX_Common;
using System;
using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace RX_DigiPrint.Helpers
{
    public class MvtTextBox : TextBox
    {
        //--- Constructor ----------------------------
        public MvtTextBox()
        {
            Style = Application.Current.Resources["MvtTextBox"] as Style;
            PreviewTouchUp += MvtTextBox_TouchUp;           
            PreviewMouseUp += MvtTextBox_MouseUp;
        }

        //--- OpenPad -----------------------------
        public void OpenPad()
        {
            MvtTextBox_TouchUp(null, null);
        }

        //--- MvtTextBox_MouseUp --------------------------------------
        void MvtTextBox_MouseUp(object sender, MouseButtonEventArgs e)
        {
            if (!IsPadActive) MvtTextBox_TouchUp(sender, null);
        }

        //--- Property IsPadActive ------------------------------------------------
        public bool IsPadActive
        {
            get { return (bool)GetValue(IsPadActiveProperty); }
            set { SetValue(IsPadActiveProperty, value); }
        }

        // Using a DependencyProperty as the backing store for IsPadActive.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty IsPadActiveProperty =
            DependencyProperty.Register("IsPadActive", typeof(bool), typeof(MvtTextBox), new PropertyMetadata(false));

        public void MvtTextBox_TouchUp(object sender, TouchEventArgs e)
        {
            IsPadActive = true;
            MvtTextPad pad = new MvtTextPad(this);
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
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.Message);
                        return;
                    }
                }
            }
        }

    }
}
