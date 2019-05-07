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
    public partial class TextPropertiesPage : Page
    {
        private TextProperties _TextBoxC;
        private int _BoxNo;

        //--- TextPropertiesPage ----------------------------------------
        public TextPropertiesPage(int boxNo, TextProperties textBoxC)
        {
            InitializeComponent();

            _BoxNo            = boxNo;
            TextBoxClass     = textBoxC;
            this.DataContext = TextBoxClass;
        }

        //--- Page_Loaded -----------------------------------------------
        private void Page_Loaded(object sender, RoutedEventArgs e)
        {
            Measure(new Size(200, 800));
            Console.WriteLine("Page_Loaded Height={0}", DesiredSize.Height);
        }

        //--- Property BoxNo ---------------------------------------
        public int BoxNo
        {
            get { return _BoxNo; }
        }

        //--- TextBoxClass ----------------------------------------------
        public TextProperties TextBoxClass
        {
            get { return _TextBoxC; }
            set { _TextBoxC = value; }
        }
   }
}
