using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Imaging;

namespace RX_Common
{
    public partial class RxMessageBox : Window
    {
        
        //--- YesNo ------------------------------------------------
        static public bool YesNo(string title, string text, MessageBoxImage image, bool defaultResult)
        {     
            RxMessageBox box = new RxMessageBox(title, text, image, defaultResult);
            bool ret = (bool)box.ShowDialog(); 
            return ret;
        }

        //--- constructor ------------------------
        public RxMessageBox(string title, string text, MessageBoxImage image, bool defaultResult)
        {
            InitializeComponent();
            CenterWindowOnScreen();
            MsgTitle.Text   = title;
            Text.Text       = text;
            
            // Icons Source: http://www.iconsdb.com/
            switch (image)
            {
                case MessageBoxImage.Warning:       Image.Source = (BitmapImage)FindResource("Warning_ico");  break;
                case MessageBoxImage.Error:         Image.Source = (BitmapImage)FindResource("Error_ico"); break;
                case MessageBoxImage.Information:   Image.Source = (BitmapImage)FindResource("Information_ico"); break;
                case MessageBoxImage.Question:      Image.Source = (BitmapImage)FindResource("Question_ico"); break;
                default: break;
            }
            if (defaultResult) Button_Yes.Focus();
            else               Button_No.Focus();
        }

        //--- CenterWindowOnScreen -----------------------
        private void CenterWindowOnScreen()
        {
            double screenWidth = System.Windows.SystemParameters.PrimaryScreenWidth;
            double screenHeight = System.Windows.SystemParameters.PrimaryScreenHeight;
            double windowWidth = this.Width;
            double windowHeight = this.Height;
            this.Left = (screenWidth / 2) - (windowWidth / 2);
            this.Top = (screenHeight / 2) - (windowHeight / 2);
        }

        //--- Yes_Clicked -----------------------------------------------
        private void Yes_Clicked(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }

        //--- No_Clicked -------------------------------------------------
        private void No_Clicked(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }
    }
}
