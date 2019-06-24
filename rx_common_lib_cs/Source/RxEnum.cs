
using System.Windows.Controls;
using System.Windows.Media.Imaging;
namespace RX_Common
{
    public class RxEnum<type> : RxBindable
    {
        public RxEnum()
        {
        }

        public RxEnum(type value, string display)
        {
            Value = value;
            Display = display;
        }

        public RxEnum(type value, string display, BitmapImage image)
        {
            Value = value;
            Display = display;
            Image = image;
        }

        private type _Value;
        public type Value
        {
            get { return _Value; }
            set { SetProperty<type>(ref _Value, value); }
        }

        private string _Display;
        public string Display
        {
            get { return _Display; }
            set { SetProperty<string>(ref _Display, value); }
        }

        private BitmapImage _Image;
        public BitmapImage Image
        {
            get { return _Image; }
            set { SetProperty<BitmapImage>(ref _Image, value); }
        }


    }
}
