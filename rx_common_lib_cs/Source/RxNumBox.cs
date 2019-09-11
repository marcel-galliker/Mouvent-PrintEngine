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
            LostFocus += RxNumBox_LostFocus;
         //   PreviewMouseUp += RxNumBox_MouseUp;
        }

        void RxNumBox_LostFocus(object sender, RoutedEventArgs e)
        {
            double min=Min;
            double max=Max;
            double val = Rx.StrToDouble(Text);
            if (min!=double.NaN && val<min) val = min;
            else if (max!=double.NaN && val>max) val = max;
            if (IsInt) Text=((int)val).ToString();
            else       Text=val.ToString();
            e.Handled = true;
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
        
        //--- Property IsInt ------------------------------------------------
        public bool IsInt
        {
            get { return (bool)GetValue(IsIntProperty); }
            set { SetValue(IsIntProperty, value); }
        }        

        // Using a DependencyProperty as the backing store for IsPadActive.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty IsIntProperty =
            DependencyProperty.Register("IsInt", typeof(bool), typeof(RxNumBox), new PropertyMetadata(false));

        //--- Property Min ------------------------------------------------
        public double Min
        {
            get { return (double)GetValue(MinProperty); }
            set { SetValue(MinProperty, value); }
        }        

        // Using a DependencyProperty as the backing store for IsPadActive.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty MinProperty =
            DependencyProperty.Register("Min", typeof(double), typeof(RxNumBox), new PropertyMetadata(double.NaN));

        //--- Property Max ------------------------------------------------
        public double Max
        {
            get { return (double)GetValue(MaxProperty); }
            set { SetValue(MaxProperty, value); }
        }        

        // Using a DependencyProperty as the backing store for IsPadActive.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty MaxProperty =
            DependencyProperty.Register("Max", typeof(double), typeof(RxNumBox), new PropertyMetadata(double.NaN));

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
            RxNumBox test=sender as RxNumBox;
            IsPadActive = true;
            RxNumPad pad = new RxNumPad(this);
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
