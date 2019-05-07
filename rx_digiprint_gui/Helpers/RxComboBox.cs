using System;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Input;
using RX_Common;

namespace RX_DigiPrint.Helpers
{
    public class RxComboBox : ComboBox
    {
        private static RxPopup _Popup = new RxPopup();

        public RxComboBox()
        {
            EventManager.RegisterClassHandler(typeof(Window), Mouse.MouseDownEvent, new MouseButtonEventHandler(OnGlobalMousepDown), true);

            //--- default values ------------------------
            IsEditable=false;
            SelectedValuePath="Value";
            TextSize=(double)FindResource("FontSize");
            Style   =(Style)FindResource("ImgComboBox");

            PreviewMouseDown        += RxComboBox_PreviewMouseDown;
            PreviewStylusButtonDown += RxComboBox_PreviewStylusButtonDown;
            IsVisibleChanged += RxComboBox_IsVisibleChanged;
        }

        void RxComboBox_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue)
            {
                _set_template(FontSize);
            }
        }

        //--- property ImageBox: ImageBox -------------------------------

        public bool ImageBox
        {
            get { return (bool)GetValue(ImageBoxProperty); }
            set { SetValue(ImageBoxProperty, value); }
        }

        // Using a DependencyProperty as the backing store for ImageBox.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty ImageBoxProperty =
            DependencyProperty.Register("ImageBox", typeof(bool), typeof(RxComboBox), new PropertyMetadata(false));

        //--- RxComboBox_PreviewMouseDown ------------------------------------
        void RxComboBox_PreviewMouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            if (!IsReadOnly)
            {
                Control ctrl = sender as Control;
                _Popup.IsOpen=false;
                _Popup.PlacementTarget=ctrl;
                if (RxScreen.Screen.Surface) _Popup.VerticalOffset = ctrl.ActualHeight*RxScreen.Screen.Scale;
                if (Items.Count>0)
                {          
                    ListView list = new ListView();
                    if (ctrl!=null) list.MinWidth = ctrl.ActualWidth;
                    list.ItemsSource = Items;

                    //--- Create the list template: a TextBox.Text="{Binding Display}" -----
                    DataTemplate template = new DataTemplate();
                    template.DataType = Items[0].GetType();
                    
                    if (ImageBox)
                    {
                        FrameworkElementFactory img = new FrameworkElementFactory(typeof(Image));
                        img.SetBinding(Image.SourceProperty, new Binding("Image"));
                        img.SetValue(TextBlock.MarginProperty, new Thickness(0,4,0,4));

                        template.VisualTree = img;
                    }
                    else
                    {
                        FrameworkElementFactory text = new FrameworkElementFactory(typeof(TextBlock));
                        text.SetBinding(TextBlock.TextProperty, new Binding("Display"));
                        double fontSize = (double)FindResource("FontSize");
                        text.SetValue(TextBlock.FontFamilyProperty, this.FontFamily);            
                        text.SetValue(TextBlock.FontSizeProperty, fontSize);            
                        text.SetValue(TextBlock.MarginProperty, new Thickness(0,4,0,4));

                        template.VisualTree = text;
                    }
                                        
                    list.ItemTemplate = template;
                    //list.HorizontalContentAlignment=HorizontalContentAlignment;
                    list.HorizontalContentAlignment=HorizontalAlignment.Center;

                    list.SelectionChanged +=popup_SelectionChanged;
                        
                    _Popup.Child=list;
                    _Popup.Open(this);
                }
            }
            if (e!=null) e.Handled = true;
        }

        //--- RxComboBox_PreviewStylusButtonDown ----------------------
        private void RxComboBox_PreviewStylusButtonDown(object sender, StylusButtonEventArgs e)
        {
            RxComboBox_PreviewMouseDown(sender, null);
            e.Handled = true;
        }

        //--- popup_SelectionChanged -----------------------------------
        private void popup_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ListView list=sender as ListView;
            SelectedIndex = list.SelectedIndex;
            RxBindable.InvokeDelayed(10, ()=>
            {
                if (_Popup!=null) _Popup.IsOpen = false;
            }
            );
            e.Handled=true;
        }        

        //--- OnGlobalMousepDown ---------------------------
        private void OnGlobalMousepDown(object sender, MouseButtonEventArgs e)
        {
            _Popup.IsOpen = false;
            e.Handled = true;
        }
       
        //--- _set_template -------------------------------
        private void _set_template(double size) 
        {
            DataTemplate template = new DataTemplate();

            if (Items.Count==0) template.DataType = typeof(RX_Common.RxEnum<int>);
            else                template.DataType = Items[0].GetType();
            
            if (ImageBox)
            {
                FrameworkElementFactory img = new FrameworkElementFactory(typeof(Image));
                img.SetBinding(Image.SourceProperty, new Binding("Image"));
                template.VisualTree = img;
            }
            else
            {
                FrameworkElementFactory text = new FrameworkElementFactory(typeof(TextBlock));
                text.SetBinding(TextBlock.TextProperty, new Binding("Display"));
                text.SetValue  (TextBlock.FontSizeProperty, size);
                template.VisualTree = text;
            }
            
            this.ItemTemplate = template;        
        }

        //--- Property TextSize ---------------------------------------
        public double TextSize
        {
            set { FontSize=value; _set_template(value); }
            get { return FontSize;}
        }
 
    }
}
