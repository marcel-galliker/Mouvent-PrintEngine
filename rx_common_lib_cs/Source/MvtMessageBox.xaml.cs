using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Imaging;

namespace RX_Common
{
    public partial class MvtMessageBox : Window, INotifyPropertyChanged
    {
        private static bool _Cancel = false;
        public event PropertyChangedEventHandler PropertyChanged;

        public enum EPurgeResult
        {
            PurgeResultUndef,
            PurgeResultCancel,
            PurgeResultYes,
            PurgeResultAll,
        };

        private string _Title;
        public string MsgBoxTitle
        {
            get { return _Title; }
            set { _Title = value; OnPropertyChanged("MsgBoxTitle"); }
        }

        private EPurgeResult _PurgeResult;
        public EPurgeResult PurgeResult
        {
            get { return _PurgeResult; }
            private set { _PurgeResult = value; OnPropertyChanged("PurgeResult"); }

        }

        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        //--- YesNoPos -------------------------------------
        static public bool YesNoPos(string title, string text, MessageBoxImage image, bool defaultResult, Point pos)
        {
            MvtMessageBox box = new MvtMessageBox(title, text, image, defaultResult);
            box.Button_Question.Visibility = Visibility.Collapsed;
            box.Button_PurgeAll.Visibility = Visibility.Collapsed;
            if (pos!=null && pos.X>=0)
            {
                box.Left=pos.X;
                box.Top=pos.Y;
            }

            bool ret = (bool)box.ShowDialog();
            return ret;
        }

        //--- YesNo ------------------------------------------------
        static public bool YesNo(string title, string text, MessageBoxImage image, bool defaultResult)
        {
            MvtMessageBox box = new MvtMessageBox(title, text, image, defaultResult);
            box.Button_Question.Visibility = Visibility.Collapsed;
            box.Button_PurgeAll.Visibility = Visibility.Collapsed;
            bool ret = (bool)box.ShowDialog();
            return ret;
        }

        //--- Information ------------------------------------------------
        static public bool Information(string title, string text)
        {
            MvtMessageBox box = new MvtMessageBox(title, text, MessageBoxImage.Information, true);
            box.Button_Question.Visibility = Visibility.Collapsed;
            box.Button_No.Visibility = Visibility.Collapsed;
            box.Button_PurgeAll.Visibility = Visibility.Collapsed;
            bool ret = (bool)box.ShowDialog();
            return ret;
        }

        static public void InformationAndExit(string title, string text,
            [System.Runtime.CompilerServices.CallerFilePath] string sourceFilePath = "",
            [System.Runtime.CompilerServices.CallerLineNumber] int sourceLineNumber = 0)
        {
            string message = text + " (" + sourceFilePath + ", Line: " + sourceLineNumber + ")";
            MvtMessageBox box = new MvtMessageBox(title, message, MessageBoxImage.Information, true);
            box.Button_Question.Visibility = Visibility.Collapsed;
            box.Button_No.Visibility = Visibility.Collapsed;
            box.Button_Yes.Visibility = Visibility.Collapsed;
            box.Button_PurgeAll.Visibility = Visibility.Collapsed;
            System.Environment.Exit(-1);
        }

        //--- YesNoCancel ------------------------------------------------
        static public bool? YesNoCancel(string title, string text, MessageBoxImage image, bool defaultResult)
        {
            _Cancel = false;
            MvtMessageBox box = new MvtMessageBox(title, text, image, defaultResult);
            box.Button_Question.Visibility = Visibility.Visible;
            box.Button_PurgeAll.Visibility = Visibility.Collapsed;
            bool? ret = box.ShowDialog();
            if (_Cancel) return null;
            return ret;
        }

        static public EPurgeResult Purge(string title, Boolean allowAll, string text)
        {
            MvtMessageBox box = new MvtMessageBox(title, text, MessageBoxImage.Question, false);
            box.Button_Question.Visibility = Visibility.Collapsed;
            box.Button_PurgeAll.Visibility = allowAll ? Visibility.Visible : Visibility.Collapsed;
            box.ShowDialog();
            return box.PurgeResult;
        }

        //--- constructor ------------------------
        public MvtMessageBox(string title, string text, MessageBoxImage image, bool defaultResult)
        {
            InitializeComponent();
            CenterWindowOnScreen();
            DataContext = this;
            MsgBoxTitle = title;
            Text.Text = text;
            PurgeResult = EPurgeResult.PurgeResultUndef;

            // Icons Source: http://www.iconsdb.com/
            switch (image)
            {
                case MessageBoxImage.Warning: Image.Source = (BitmapImage)FindResource("Warning_ico"); break;
                case MessageBoxImage.Error: Image.Source = (BitmapImage)FindResource("Error_ico"); break;
                case MessageBoxImage.Information: Image.Source = (BitmapImage)FindResource("Information_ico"); break;
                case MessageBoxImage.Question: Image.Source = (BitmapImage)FindResource("Question_ico"); break;
                default: break;
            }
            if (defaultResult) Button_Yes.Focus();
            else Button_No.Focus();
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
            PurgeResult = EPurgeResult.PurgeResultYes;
            DialogResult = true;
        }

        //--- No_Clicked -------------------------------------------------
        private void No_Clicked(object sender, RoutedEventArgs e)
        {
            PurgeResult = EPurgeResult.PurgeResultCancel;
            DialogResult = false;
        }

        //--- Question_Clicked -------------------------------------------------
        private void Question_Clicked(object sender, RoutedEventArgs e)
        {
            _Cancel = true;
            PurgeResult = EPurgeResult.PurgeResultUndef;
            Close();
        }

        private void PurgeAll_Clicked(object sender, RoutedEventArgs e)
        {
            PurgeResult = EPurgeResult.PurgeResultAll;
            Close();
        }
    }
}
