using RX_Common;
using System;
using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Resources;
using System.Windows.Threading;

namespace RX_LabelComposer.Common
{
    public class RxNumBox : TextBox
    {
        //--- Constructor ----------------------------
        public RxNumBox()
        {
            //Style = Application.Current.Resources["RxNumBox"] as Style; // Commented by samuel
            PreviewTouchUp += RxNumBox_TouchUp;
        //  TouchUp += RxNumBox_TouchUp;
            KeyDown   += OnKeyDown;
        //    PreviewMouseUp += RxNumBox_MouseUp;
        }

        //--- RxNumBox_MouseUp ------------------------------
        void RxNumBox_MouseUp(object sender, MouseButtonEventArgs e)
        {
            if (!IsPadActive) RxNumBox_TouchUp(sender, null);
        }

        //--- Property IsPadActive ------------------------------------------------
        public bool IsPadActive
        {
            get { return (bool)GetValue(IsPadActiveProperty); }
            set { SetValue(IsPadActiveProperty, value); }
        }

        // Using a DependencyProperty as the backing store for IsPadActive.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty IsPadActiveProperty =
            DependencyProperty.Register("IsPadActive", typeof(bool), typeof(RxNumBox), new PropertyMetadata(false));
        
        //--- RxNumBox_TouchUp -----------------------------------
        void RxNumBox_TouchUp(object sender, TouchEventArgs e)
        {
            IsPadActive = true;
            // Comment made by Samuel de Santis on June, 7th 2018.
            // I had to set the second parameter to false but don't know why.
            RxNumPad pad = new RxNumPad(this, false);
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
                        Text = d.ToString(CultureInfo.InvariantCulture);
                        return;
                    }
                }

                if (Text.Equals("####"))
                {
                    Text =  "";
                    e.Handled=true;
                    return;
                }          

            }
        }

        //--- Property Value ---------------------------------------
        public double Value
        {
            get { return Rx.StrToDouble(Text);}
        }
        
    }
}
