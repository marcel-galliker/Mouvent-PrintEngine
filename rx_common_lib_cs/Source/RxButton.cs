using System;
using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Interop;

namespace RX_Common
{
    public class RxButton : Button
    {
        public RxButton()
        {
            IsChecked = false;
            Loaded += RxButton_Loaded;

            PreviewMouseLeftButtonUp    += RxButton_PreviewMouseLeftButtonUp;            
            PreviewMouseDown            += RxButton_PreviewMouseDown;
            
            #if DEBUG
            // --- debugging events ---
            PreviewStylusButtonDown     += RxButton_PreviewStylusButtonDown;
            PreviewStylusButtonUp       += RxButton_PreviewStylusButtonUp;
            PreviewStylusDown           += RxButton_PreviewStylusDown;
            PreviewStylusInAirMove      += RxButton_PreviewStylusInAirMove;
            PreviewStylusInRange        += RxButton_PreviewStylusInRange;
            PreviewStylusMove           += RxButton_PreviewStylusMove;
            PreviewStylusOutOfRange     += RxButton_PreviewStylusOutOfRange;
            PreviewStylusSystemGesture  += RxButton_PreviewStylusSystemGesture;
            PreviewStylusUp             += RxButton_PreviewStylusUp;

            StylusButtonDown            += RxButton_StylusButtonDown;
            StylusButtonUp              += RxButton_StylusButtonUp;
            StylusDown                  += RxButton_StylusDown;
            StylusEnter                 += RxButton_StylusEnter;
            StylusInAirMove             += RxButton_StylusInAirMove;
            StylusInRange               += RxButton_StylusInRange;
            StylusLeave                 += RxButton_StylusLeave;
            StylusMove                  += RxButton_StylusMove;
            StylusOutOfRange            += RxButton_StylusOutOfRange;
            StylusSystemGesture         += RxButton_StylusSystemGesture;
            StylusUp                    += RxButton_StylusUp;

            TouchDown                   += RxButton_TouchDown;
            TouchEnter                  += RxButton_TouchEnter;
            TouchLeave                  += RxButton_TouchLeave;
            TouchMove                   += RxButton_TouchMove;
            TouchUp                     += RxButton_TouchUp;

            MouseDown                   += RxButton_MouseDown;

            Click                       += RxButton_Click;
            
            #endif
        }

        //--- RxButton_Loaded ---------------------------
        private void RxButton_Loaded(object sender, System.EventArgs e)
        {
            HwndSource source = PresentationSource.FromVisual(this) as HwndSource;
            if (source!=null) source.AddHook(WndProc);
        }

        //--- _OnButton -----------------------------------------------
        private bool _OnButton(int x, int y)
        {
            if (!this.IsHitTestVisible) return false;
            if (!this.IsVisible) return false;
            if (!this.IsEnabled) return false;
            if (!TouchEnabled && !this.IsMouseDirectlyOver) return false;

            Point min = this.PointToScreen(new Point(0,0));
            Point max = new Point(min.X+this.ActualWidth/RxScreen.Screen.Scale, min.Y+this.ActualHeight/RxScreen.Screen.Scale);

            return (min.X<=x && x<=max.X && min.Y<=y && y<=max.Y);
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
        private bool _TouchIsDown=false;
        private IntPtr WndProc(System.IntPtr hwnd, int msg, System.IntPtr wParam, System.IntPtr lParam, ref bool handled)
        {            
            // message numbers defined in WinUser.h 

            // Debug.WriteLine("msg=0x{1:x}", this.Name, msg);
            int x, y;

            try
            {
                x = (int) ((Int64)lParam)&0xffff;
                y = (int)(((Int64)lParam)>>16)&0xffff;
            }
            catch(Exception)
            {
                return IntPtr.Zero;
            }
            
            switch (msg)
            {
                case 0x0245:   // WM_POINTERUPDATE
//                                handled = _handle_touch_event(x, y, PreviewMouseDownEvent);
                                if (_TouchIsDown && !_OnButton(x,y)) 
                                {
                                    handled = _raise_event(PreviewMouseUpEvent);
                                    _TouchIsDown = false;                                 
                                }
                                break;

                case 0x0246: // WM_POINTERDOWN

                                /*
                                int flags    = (((int)wParam)>>16) & 0xffff;
                                int id       = ((int)wParam) & 0xffff;

                                Point pos = this.PointToScreen(new Point(0,0));
                                Point max = new Point(pos.X+this.ActualWidth/RxScreen.Screen.Scale, pos.Y+this.ActualHeight/RxScreen.Screen.Scale);

                                Debug.WriteLine("WM_POINTERDOWN (wParam=0x{1:X}  lParam=0x{2:X}, x={3} y={4})", this.Name, wParam, lParam, xPos, yPos);
                                Debug.WriteLine("WM_POINTERDOWN (id={1})", this.Name, id);

                                Debug.WriteLine("Position: ({1}, {2}) w={3} h={4}", this.Name, pos.X, pos.Y, this.ActualWidth, this.ActualHeight);
                                Debug.WriteLineIf((flags & 0x00000001)!=0, "WM_POINTERDOWN.POINTER_MESSAGE_FLAG_NEW",           this.Name);
                                Debug.WriteLineIf((flags & 0x00000002)!=0, "WM_POINTERDOWN.POINTER_MESSAGE_FLAG_INRANGE",       this.Name);
                                Debug.WriteLineIf((flags & 0x00000004)!=0, "WM_POINTERDOWN.POINTER_MESSAGE_FLAG_INCONTACT",     this.Name);
                                Debug.WriteLineIf((flags & 0x00000010)!=0, "WM_POINTERDOWN.POINTER_MESSAGE_FLAG_FIRSTBUTTON",   this.Name);
                                Debug.WriteLineIf((flags & 0x00000020)!=0, "WM_POINTERDOWN.POINTER_MESSAGE_FLAG_SECONDBUTTON",  this.Name);
                                Debug.WriteLineIf((flags & 0x00000040)!=0, "WM_POINTERDOWN.POINTER_MESSAGE_FLAG_THIRDBUTTON",   this.Name);
                                Debug.WriteLineIf((flags & 0x00000080)!=0, "WM_POINTERDOWN.POINTER_MESSAGE_FLAG_FOURTHBUTTON",  this.Name);
                                Debug.WriteLineIf((flags & 0x00000100)!=0, "WM_POINTERDOWN.POINTER_MESSAGE_FLAG_FIFTHBUTTON",   this.Name);
                                Debug.WriteLineIf((flags & 0x00002000)!=0, "WM_POINTERDOWN.POINTER_MESSAGE_FLAG_PRIMARY",       this.Name);
                                Debug.WriteLineIf((flags & 0x00004000)!=0, "WM_POINTERDOWN.POINTER_MESSAGE_FLAG_CONFIDENCE",    this.Name);
                                Debug.WriteLineIf((flags & 0x00008000)!=0, "WM_POINTERDOWN.POINTER_MESSAGE_FLAG_CANCELED",      this.Name);

                                Debug.WriteLine("HitX = {1} ({2} < {3} < {4})", this.Name, pos.X<=x && xPos<=max.X, pos.X, x, max.X);
                                Debug.WriteLine("HitY = {1} ({2} < {3} < {4})", this.Name, pos.Y<=y && yPos<=max.Y, pos.Y, y, max.Y);
                                */
                                
                                if (_OnButton(x,y))
                                {
                                    if (TouchEnabled)
                                    { 
                                        _TouchIsDown = true;
                                        handled = _raise_event(PreviewMouseDownEvent);
                                    }
                                    else
                                    {
                                        RaiseEvent(new RoutedEventArgs(ButtonBase.ClickEvent, this));
                                        handled=true;
                                    }
                                }

                        break;

                case 0x0247: // WM_POINTERUP
                                if (_OnButton(x,y)) 
                                {
                                    _TouchIsDown = false; 
                                    handled = _raise_event(PreviewMouseUpEvent);
                                }
                                break;
            }
            return IntPtr.Zero;
        }

        private void RxButton_MouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e) {Debug.WriteLine("***MouseDown", this.Name);}
        
        void RxButton_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (TouchEnabled)
            { 
                _TouchIsDown = true;
                PreviewMouseDown -= RxButton_PreviewMouseDown;
                e.Handled = _raise_event(PreviewMouseDownEvent);
                PreviewMouseDown += RxButton_PreviewMouseDown;
            }
            else
            {
                RaiseEvent(new RoutedEventArgs(ButtonBase.ClickEvent, this));
                e.Handled=true;
            }
        }

        #if DEBUG
        void RxButton_PreviewStylusUp           (object sender, System.Windows.Input.StylusEventArgs e)             {Debug.WriteLine("PreviewStylusUp", this.Name);}
        void RxButton_PreviewStylusSystemGesture(object sender, System.Windows.Input.StylusSystemGestureEventArgs e){Debug.WriteLine("PreviewStylusSystemGesture", this.Name);}
        void RxButton_PreviewStylusOutOfRange   (object sender, System.Windows.Input.StylusEventArgs e)             {Debug.WriteLine("PreviewStylusOutOfRange", this.Name);}
        void RxButton_PreviewStylusMove         (object sender, System.Windows.Input.StylusEventArgs e)             {Debug.WriteLine("PreviewStylusMove", this.Name);}
        void RxButton_PreviewStylusInRange      (object sender, System.Windows.Input.StylusEventArgs e)             {Debug.WriteLine("PreviewStylusInRange", this.Name);}
        void RxButton_PreviewStylusInAirMove    (object sender, System.Windows.Input.StylusEventArgs e)             {Debug.WriteLine("PreviewStylusInAirMove", this.Name);}
        void RxButton_PreviewStylusDown         (object sender, System.Windows.Input.StylusDownEventArgs e)         {Debug.WriteLine("PreviewStylusDown", this.Name);}
        void RxButton_PreviewStylusButtonUp     (object sender, System.Windows.Input.StylusButtonEventArgs e)       {Debug.WriteLine("PreviewStylusButtonUp", this.Name);}
        void RxButton_PreviewStylusButtonDown   (object sender, System.Windows.Input.StylusButtonEventArgs e)       {Debug.WriteLine("PreviewStylusButtonDown", this.Name);}

        void RxButton_StylusButtonDown          (object sender, System.Windows.Input.StylusButtonEventArgs e)       {Debug.WriteLine("StylusButtonDown", this.Name);}
        void RxButton_StylusUp                  (object sender, System.Windows.Input.StylusEventArgs e)             {Debug.WriteLine("StylusUp", this.Name);}
        void RxButton_StylusSystemGesture       (object sender, System.Windows.Input.StylusSystemGestureEventArgs e){Debug.WriteLine("StylusSystemGesture", this.Name);}
        void RxButton_StylusOutOfRange          (object sender, System.Windows.Input.StylusEventArgs e)             {Debug.WriteLine("StylusOutOfRange", this.Name);}
        void RxButton_StylusMove                (object sender, System.Windows.Input.StylusEventArgs e)             {Debug.WriteLine("StylusMove", this.Name);}
        void RxButton_StylusLeave               (object sender, System.Windows.Input.StylusEventArgs e)             {Debug.WriteLine("StylusLeave", this.Name);}
        void RxButton_StylusInRange             (object sender, System.Windows.Input.StylusEventArgs e)             {Debug.WriteLine("StylusInRange", this.Name);}
        void RxButton_StylusInAirMove           (object sender, System.Windows.Input.StylusEventArgs e)             {Debug.WriteLine("StylusInAirMove", this.Name);}
        void RxButton_StylusEnter               (object sender, System.Windows.Input.StylusEventArgs e)             {Debug.WriteLine("StylusEnter", this.Name);}
        void RxButton_StylusButtonUp            (object sender, System.Windows.Input.StylusButtonEventArgs e)       {Debug.WriteLine("StylusButtonUp", this.Name);}
        void RxButton_StylusDown                (object sender, System.Windows.Input.StylusDownEventArgs e)         {Debug.WriteLine("StylusDown", this.Name);}

        void RxButton_Click                     (object sender, RoutedEventArgs e)                                  {Debug.WriteLine("Click", this.Name);}

        void RxButton_TouchMove                 (object sender, System.Windows.Input.TouchEventArgs e)              {Debug.WriteLine("TouchMove", this.Name);}
        void RxButton_TouchLeave                (object sender, System.Windows.Input.TouchEventArgs e)              {Debug.WriteLine("TouchLeave", this.Name);}
        void RxButton_TouchEnter                (object sender, System.Windows.Input.TouchEventArgs e)              {Debug.WriteLine("TouchEnter", this.Name);}
        void RxButton_TouchDown                 (object sender, System.Windows.Input.TouchEventArgs e)              {Debug.WriteLine("TouchDown", this.Name);}
        void RxButton_TouchUp                   (object sender, System.Windows.Input.TouchEventArgs e)              {Debug.WriteLine("TouchUp", this.Name);}

        #endif

        //--- Property TouchEnabled ---------------------------------------
        private bool _TouchEnabled=false;
        public bool   TouchEnabled
        {
            get { return _TouchEnabled; }
            set { _TouchEnabled = value; }
        }
        

        private void RxButton_PreviewMouseLeftButtonUp(object sender, System.Windows.Input.MouseButtonEventArgs e)  
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

        //--- Property IsChecked ---------------------
        public bool IsChecked   
        {
            get { return (bool)GetValue(IsButtonCheckedProperty); }
            set { SetValue(IsButtonCheckedProperty, value); }
        }

        // Using a DependencyProperty as the backing store for IsButtonChecked.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty IsButtonCheckedProperty =
            DependencyProperty.Register("IsChecked", typeof(bool), typeof(RxButton));               

        //--- Property IsBusy ---------------------
        public bool IsBusy   
        {
            get { return (bool)GetValue(IsButtonBusyProperty); }
            set { SetValue(IsButtonBusyProperty, value); }
        }

        // Using a DependencyProperty as the backing store for IsButtonChecked.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty IsButtonBusyProperty =
            DependencyProperty.Register("IsBusy", typeof(bool), typeof(RxButton));               
    }
}
