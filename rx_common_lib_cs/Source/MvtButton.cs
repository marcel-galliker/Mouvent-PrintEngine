using System;
using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Interop;

namespace RX_Common
{
    public class MvtButton : Button
    {
        public MvtButton()
        {
            IsChecked = false;
            Loaded += MvtButton_Loaded;

            PreviewMouseLeftButtonUp += MvtButton_PreviewMouseLeftButtonUp;
            PreviewMouseDown += MvtButton_PreviewMouseDown;

#if DEBUG
            // --- debugging events ---
            PreviewStylusButtonDown += MvtButton_PreviewStylusButtonDown;
            PreviewStylusButtonUp += MvtButton_PreviewStylusButtonUp;
            PreviewStylusDown += MvtButton_PreviewStylusDown;
            PreviewStylusInAirMove += MvtButton_PreviewStylusInAirMove;
            PreviewStylusInRange += MvtButton_PreviewStylusInRange;
            PreviewStylusMove += MvtButton_PreviewStylusMove;
            PreviewStylusOutOfRange += MvtButton_PreviewStylusOutOfRange;
            PreviewStylusSystemGesture += MvtButton_PreviewStylusSystemGesture;
            PreviewStylusUp += MvtButton_PreviewStylusUp;

            StylusButtonDown += MvtButton_StylusButtonDown;
            StylusButtonUp += MvtButton_StylusButtonUp;
            StylusDown += MvtButton_StylusDown;
            StylusEnter += MvtButton_StylusEnter;
            StylusInAirMove += MvtButton_StylusInAirMove;
            StylusInRange += MvtButton_StylusInRange;
            StylusLeave += MvtButton_StylusLeave;
            StylusMove += MvtButton_StylusMove;
            StylusOutOfRange += MvtButton_StylusOutOfRange;
            StylusSystemGesture += MvtButton_StylusSystemGesture;
            StylusUp += MvtButton_StylusUp;

            TouchDown += MvtButton_TouchDown;
            TouchEnter += MvtButton_TouchEnter;
            TouchLeave += MvtButton_TouchLeave;
            TouchMove += MvtButton_TouchMove;
            TouchUp += MvtButton_TouchUp;

            MouseDown += MvtButton_MouseDown;

            Click += MvtButton_Click;

#endif
        }

        private void MvtButton_Loaded(object sender, System.EventArgs e)
        {
            HwndSource source = PresentationSource.FromVisual(this) as HwndSource;
            if (source != null) source.AddHook(WndProc);
        }

        //--- _OnButton -----------------------------------------------
        private bool _OnButton(int x, int y)
        {
            if (!this.IsHitTestVisible) return false;
            if (!this.IsVisible) return false;
            if (!this.IsEnabled) return false;
            if (!TouchEnabled && !this.IsMouseDirectlyOver) return false;

            Point min = this.PointToScreen(new Point(0, 0));
            Point max = new Point(min.X + this.ActualWidth / RxScreen.Screen.Scale, min.Y + this.ActualHeight / RxScreen.Screen.Scale);

            return (min.X <= x && x <= max.X && min.Y <= y && y <= max.Y);
        }

        //--- _raise_event -----------------
        private bool _raise_event(RoutedEvent e)
        {
            MouseButtonEventArgs args = new MouseButtonEventArgs(Mouse.PrimaryDevice, Environment.TickCount, MouseButton.Left)
            {
                RoutedEvent = e,
                Source = this,
            };
            RaiseEvent(args);
            return true;
        }

        //--- WndProc ---------------------------------
        private bool _TouchIsDown = false;
        private IntPtr WndProc(System.IntPtr hwnd, int msg, System.IntPtr wParam, System.IntPtr lParam, ref bool handled)
        {
            // message numbers defined in WinUser.h 

            // Debug.WriteLine("msg=0x{1:x}", this.Name, msg);
            int x, y;

            try
            {
                x = (int)((Int64)lParam) & 0xffff;
                y = (int)(((Int64)lParam) >> 16) & 0xffff;
            }
            catch (Exception)
            {
                return IntPtr.Zero;
            }

            switch (msg)
            {
                case 0x0245:   // WM_POINTERUPDATE
                               //                                handled = _handle_touch_event(x, y, PreviewMouseDownEvent);
                    if (_TouchIsDown && !_OnButton(x, y))
                    {
                        handled = _raise_event(PreviewMouseUpEvent);
                        _TouchIsDown = false;
                    }
                    break;

                case 0x0246: // WM_POINTERDOWN

                    if (_OnButton(x, y))
                    {
                        if (TouchEnabled)
                        {
                            _TouchIsDown = true;
                            handled = _raise_event(PreviewMouseDownEvent);
                        }
                        else
                        {
                            RaiseEvent(new RoutedEventArgs(ButtonBase.ClickEvent, this));
                            handled = true;
                        }
                    }

                    break;

                case 0x0247: // WM_POINTERUP
                    if (_OnButton(x, y))
                    {
                        _TouchIsDown = false;
                        handled = _raise_event(PreviewMouseUpEvent);
                    }
                    break;
            }
            return IntPtr.Zero;
        }

        private void MvtButton_MouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e) { Debug.WriteLine("***MouseDown", this.Name); }

        void MvtButton_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (TouchEnabled)
            {
                _TouchIsDown = true;
                PreviewMouseDown -= MvtButton_PreviewMouseDown;
                e.Handled = _raise_event(PreviewMouseDownEvent);
                PreviewMouseDown += MvtButton_PreviewMouseDown;
            }
            else
            {
                RaiseEvent(new RoutedEventArgs(ButtonBase.ClickEvent, this));
                e.Handled = true;
            }
        }

#if DEBUG
        void MvtButton_PreviewStylusUp(object sender, System.Windows.Input.StylusEventArgs e) { Debug.WriteLine("PreviewStylusUp", this.Name); }
        void MvtButton_PreviewStylusSystemGesture(object sender, System.Windows.Input.StylusSystemGestureEventArgs e) { Debug.WriteLine("PreviewStylusSystemGesture", this.Name); }
        void MvtButton_PreviewStylusOutOfRange(object sender, System.Windows.Input.StylusEventArgs e) { Debug.WriteLine("PreviewStylusOutOfRange", this.Name); }
        void MvtButton_PreviewStylusMove(object sender, System.Windows.Input.StylusEventArgs e) { Debug.WriteLine("PreviewStylusMove", this.Name); }
        void MvtButton_PreviewStylusInRange(object sender, System.Windows.Input.StylusEventArgs e) { Debug.WriteLine("PreviewStylusInRange", this.Name); }
        void MvtButton_PreviewStylusInAirMove(object sender, System.Windows.Input.StylusEventArgs e) { Debug.WriteLine("PreviewStylusInAirMove", this.Name); }
        void MvtButton_PreviewStylusDown(object sender, System.Windows.Input.StylusDownEventArgs e) { Debug.WriteLine("PreviewStylusDown", this.Name); }
        void MvtButton_PreviewStylusButtonUp(object sender, System.Windows.Input.StylusButtonEventArgs e) { Debug.WriteLine("PreviewStylusButtonUp", this.Name); }
        void MvtButton_PreviewStylusButtonDown(object sender, System.Windows.Input.StylusButtonEventArgs e) { Debug.WriteLine("PreviewStylusButtonDown", this.Name); }

        void MvtButton_StylusButtonDown(object sender, System.Windows.Input.StylusButtonEventArgs e) { Debug.WriteLine("StylusButtonDown", this.Name); }
        void MvtButton_StylusUp(object sender, System.Windows.Input.StylusEventArgs e) { Debug.WriteLine("StylusUp", this.Name); }
        void MvtButton_StylusSystemGesture(object sender, System.Windows.Input.StylusSystemGestureEventArgs e) { Debug.WriteLine("StylusSystemGesture", this.Name); }
        void MvtButton_StylusOutOfRange(object sender, System.Windows.Input.StylusEventArgs e) { Debug.WriteLine("StylusOutOfRange", this.Name); }
        void MvtButton_StylusMove(object sender, System.Windows.Input.StylusEventArgs e) { Debug.WriteLine("StylusMove", this.Name); }
        void MvtButton_StylusLeave(object sender, System.Windows.Input.StylusEventArgs e) { Debug.WriteLine("StylusLeave", this.Name); }
        void MvtButton_StylusInRange(object sender, System.Windows.Input.StylusEventArgs e) { Debug.WriteLine("StylusInRange", this.Name); }
        void MvtButton_StylusInAirMove(object sender, System.Windows.Input.StylusEventArgs e) { Debug.WriteLine("StylusInAirMove", this.Name); }
        void MvtButton_StylusEnter(object sender, System.Windows.Input.StylusEventArgs e) { Debug.WriteLine("StylusEnter", this.Name); }
        void MvtButton_StylusButtonUp(object sender, System.Windows.Input.StylusButtonEventArgs e) { Debug.WriteLine("StylusButtonUp", this.Name); }
        void MvtButton_StylusDown(object sender, System.Windows.Input.StylusDownEventArgs e) { Debug.WriteLine("StylusDown", this.Name); }

        void MvtButton_Click(object sender, RoutedEventArgs e) { Debug.WriteLine("Click", this.Name); }

        void MvtButton_TouchMove(object sender, System.Windows.Input.TouchEventArgs e) { Debug.WriteLine("TouchMove", this.Name); }
        void MvtButton_TouchLeave(object sender, System.Windows.Input.TouchEventArgs e) { Debug.WriteLine("TouchLeave", this.Name); }
        void MvtButton_TouchEnter(object sender, System.Windows.Input.TouchEventArgs e) { Debug.WriteLine("TouchEnter", this.Name); }
        void MvtButton_TouchDown(object sender, System.Windows.Input.TouchEventArgs e) { Debug.WriteLine("TouchDown", this.Name); }
        void MvtButton_TouchUp(object sender, System.Windows.Input.TouchEventArgs e) { Debug.WriteLine("TouchUp", this.Name); }

#endif

        //--- Property TouchEnabled ---------------------------------------
        private bool _TouchEnabled = false;
        public bool TouchEnabled
        {
            get { return _TouchEnabled; }
            set { _TouchEnabled = value; }
        }


        private void MvtButton_PreviewMouseLeftButtonUp(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            Debug.WriteLine("PreviewMouseLeftButtonUp IsFocused={1}", this.Name, IsFocused);
            /*
            if (!IsFocused)
            {
                this.RaiseEvent(new RoutedEventArgs(ButtonBase.ClickEvent, this));
                e.Handled=true;
            }
             * */
        }

       
        // Using a DependencyProperty as the backing store for IsButtonChecked.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty IsCheckedProperty =
            DependencyProperty.Register("IsChecked",
                typeof(bool),
                typeof(RX_Common.MvtButton));
        
        public bool IsChecked
        {
            get { return (bool)GetValue(IsCheckedProperty); }
            set { SetValue(IsCheckedProperty, value); }
        }

        //--- Property IsBusy ---------------------
        public bool IsBusy
        {
            get { return (bool)GetValue(IsBusyProperty); }
            set { SetValue(IsBusyProperty, value); }
        }

        // Using a DependencyProperty as the backing store for IsButtonChecked.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty IsBusyProperty =
            DependencyProperty.Register("IsBusy", typeof(bool), typeof(RX_Common.MvtButton));
    }
}
