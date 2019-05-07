using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Helpers
{
    public class RxGridSplitter: GridSplitter
    {
        //--- RxGridSplitter -----------------------------
        public RxGridSplitter()
        {
        //   Style: See XamGrid_Styles.xaml / "RxGridSplitter"
        }

        public string Title
        {
            get { return (string)GetValue(TitleProperty); }
            set { SetValue(TitleProperty, value); }
        }

        public static readonly DependencyProperty TitleProperty =
            DependencyProperty.Register("Title", typeof(string), typeof(RxGridSplitter), new FrameworkPropertyMetadata("*********"));
        
    }
}
