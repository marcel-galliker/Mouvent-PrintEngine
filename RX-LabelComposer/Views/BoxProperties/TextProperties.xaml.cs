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
using RX_JobComposer.Models;

namespace RX_JobComposer.Views
{
    /// <summary>
    /// Interaction logic for TextProperties.xaml
    /// </summary>
    public partial class TextPropertiesPage : Page
    {
        private TextBoxDefinition _TextDef;

        public TextPropertiesPage(TextBoxDefinition textDef)
        {
            InitializeComponent();
            TextDef = textDef;
        }

        private void Page_Loaded(object sender, RoutedEventArgs e)
        {
            //--- Data Context -----------------------------------
            this.DataContext = TextDef;
 //           Properties.DataContext = TextDef;
        }

        public TextBoxDefinition TextDef
        {
            get { return _TextDef; }
            set { _TextDef = value; }
        }

    }
}
