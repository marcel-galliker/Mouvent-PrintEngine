using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.Alignment
{
    /// <summary>
    /// Interaction logic for InstructionsView.xaml
    /// </summary>
    public partial class InstructionsView : UserControl, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private System.Windows.Media.ImageSource _imageSource;
        public System.Windows.Media.ImageSource ImageSource
        {
            get
            {
                return _imageSource;
            }
            set
            {
                _imageSource = value;
                OnPropertyChanged("ImageSource");
            }
        }

        private string _imageHeader;
        public string ImageHeader
        {
            get
            {
                return _imageHeader;
            }
            set
            {
                _imageHeader = value;
                OnPropertyChanged("ImageHeader");
            }
        }

        public InstructionsView(System.Windows.Media.ImageSource imageSource, string imageHeader)
        {
            InitializeComponent();
            DataContext = this;

            ImageSource = imageSource;
            ImageHeader = imageHeader;
        }

        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }
    }
}
