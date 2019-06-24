using ImageCircle.Forms.Plugin.Abstractions;
using RX_DigiPrint.Services;
using System;
using System.Diagnostics;
using Xamarin.Forms;

using Xamarin.Forms.Xaml;
using XLabs.Forms.Behaviors;
using XLabs.Forms.Controls;

namespace DigiPrint.Common
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class RxButton : ContentView
    {
        public RxButton()
        {
            InitializeComponent();
            State = RxBtDef.EButtonState.isEnabled;
        }

        //--- Property Click -------------------------------------------
        public static readonly BindableProperty ClickProperty = BindableProperty.Create ("Click", typeof(Action<object>), typeof(RxButton));
        public Action<object> Click 
        {
          get { return  (Action<object>)GetValue (ClickProperty);}
          set { SetValue (ClickProperty, value);}
        }

        //--- Property MouseUp -------------------------------------------
        public static readonly BindableProperty MouseUpProperty = BindableProperty.Create ("MouseUp", typeof(Action<object>), typeof(RxButton));
        public Action<object> MouseUp 
        {
          get {return  (Action<object>)GetValue (MouseUpProperty);}
          set { SetValue (MouseUpProperty, value);}
        }

        //--- handle touch events --------------------------------
        //--- Property Image ---------------------------------------
        private string _ImageName;
        public string Image
        {
            set 
            {
                _ImageName = value;
                _Image.Source = ImageSource.FromResource("DigiPrint.Resources.drawable."+value);
            }
        }

        //--- Property IsPressed ---------------------------------------
        private bool _IsPressed;
        public bool IsPressed
        {
            get { return _IsPressed; }
            set { _IsPressed = value; }
        }

        //--- Property IsLarge ---------------------------------------
        private bool? _isLarge;
        public bool IsLarge
        {
            get { return (_isLarge!=null) && (bool)_isLarge; }
            set 
            {
                if (value != _isLarge)
                {
                    _isLarge = value;
                    _Grid.HeightRequest = _Grid.WidthRequest = (bool)_isLarge ? 100 : 80;
                }
            }
        }

        //--- State Property ----------------------------------------
        public static readonly BindableProperty StateProperty =
            BindableProperty.Create("State", typeof(RxBtDef.EButtonState), typeof(RxButton), (RxBtDef.EButtonState)0, BindingMode.OneWay, null, StatePropertyChanged);

        static void StatePropertyChanged(BindableObject bindable, object oldValue, object newValue)
        {
            RxButton button = bindable as RxButton;
            if (button!=null) button.State= (RxBtDef.EButtonState)newValue;
        }

        //--- Property State ---------------------------------------
        private bool _isEnabled;
        private bool _isChecked;
        public RxBtDef.EButtonState State
        { 
            set 
            {
                _isEnabled = (value & RxBtDef.EButtonState.isEnabled)!=0;
                _isChecked = (value & RxBtDef.EButtonState.isChecked)!=0;
                IsLarge    = (value & RxBtDef.EButtonState.isLarge) != 0;
                _Image.FillColor   = _isChecked? Rx.CheckedBackground : Color.LightGray;
                _Disable.IsVisible = _Disable.IsEnabled = !_isEnabled;
            }
        }

        //--- GesturesContentView_GestureRecognized -------------------------------
        private void GesturesContentView_GestureRecognized(object sender, GestureResult e)
        {
            switch (e.GestureType)
            {
                case GestureType.Down:  Click?.Invoke(sender);   IsPressed=true;   break;
                case GestureType.Up:    MouseUp?.Invoke(sender); IsPressed=false;  break;
                default:                                                           break;
            }
        }
    }
}