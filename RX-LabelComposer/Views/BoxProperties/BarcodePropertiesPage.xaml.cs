using System;
using System.Collections.Generic;
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
using RX_LabelComposer.Models;

namespace RX_LabelComposer.Views
{
    /// <summary>
    /// Interaction logic for TextProperties.xaml
    /// </summary>
    public partial class BarcodePropertiesPage : Page
    {
        private BarcodeProperties _BcBoxC;
        private int _BoxNo;

        public BarcodePropertiesPage(int boxNo, BarcodeProperties bcBoxC)
        {
            InitializeComponent();

            BarcodeBoxClass = bcBoxC;
            this.DataContext = BarcodeBoxClass;
            _BoxNo = boxNo;
        }

        private void Page_Loaded(object sender, RoutedEventArgs e)
        {
        }

        //--- Property BoxNo ---------------------------------------
        public int BoxNo
        {
            get { return _BoxNo; }
        }
        
        public BarcodeProperties BarcodeBoxClass
        {
            get { return _BcBoxC; }
            private set {_BcBoxC = value; }
        }

        private void OnBarChanged(object sender, TextChangedEventArgs e)
        {
            TextBox s = sender as TextBox;
            if (s != null)
            {
                Binding binding = BindingOperations.GetBinding(s, TextBox.TextProperty);
                _BcBoxC.OnBarChanged(binding.Path.Path, s.Text);
            }
        }

        private void OnSpaceChanged(object sender, TextChangedEventArgs e)
        {
            TextBox s = sender as TextBox;
            if (s != null)
            {
                Binding binding = BindingOperations.GetBinding(s, TextBox.TextProperty);
                _BcBoxC.OnSpaceChanged(binding.Path.Path, s.Text);
            }
        }
    }
}
