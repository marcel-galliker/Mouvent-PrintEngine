using System;
using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace RX_Common
{
    public class RxNumBox : TextBox
    {
        //--- Constructor ----------------------------
        public RxNumBox()
        {
            Style = Application.Current.Resources["RxNumBox"] as Style;
         //   PreviewTouchUp += RxNumBox_TouchUp;
            PreviewMouseDown +=RxNumBox_PreviewMouseDown;
            TouchUp += RxNumBox_TouchUp;
            KeyDown   += OnKeyDown;
         //   PreviewMouseUp += RxNumBox_MouseUp;
        }

        //--- RxNumBox_PreviewMouseDown --------------------------------------------
        private void RxNumBox_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            // used because touch is disabled
            e.Handled=true;
            RxNumBox_TouchUp(sender, null);
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
        
        //--- Property IsUINT32 ------------------------------------------------
        public bool IsUInt32
        {
            get { return (bool)GetValue(IsUInt32Property); }
            set { SetValue(IsUInt32Property, value); }
        }        

        // Using a DependencyProperty as the backing store for IsPadActive.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty IsUInt32Property =
            DependencyProperty.Register("IsUInt32", typeof(bool), typeof(RxNumBox), new PropertyMetadata(false));

        //--- Property ShowRolls ------------------------------------------------
        public bool ShowRolls
        {
            get { return (bool)GetValue(ShowRollsProperty); }
            set { SetValue(ShowRollsProperty, value); }
        }        

        // Using a DependencyProperty as the backing store for IsPadActive.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty ShowRollsProperty =
            DependencyProperty.Register("ShowRolls", typeof(bool), typeof(RxNumBox), new PropertyMetadata(false));

        //--- RxNumBox_TouchUp -----------------------------------
        void RxNumBox_TouchUp(object sender, TouchEventArgs e)
        {
            IsPadActive = true;
            RxNumPad pad = new RxNumPad(this, IsUInt32);
            if (ShowRolls) pad.ShowRolls=true;
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
