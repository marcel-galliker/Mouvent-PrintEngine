using RX_LabelComposer.Common;
using RX_LabelComposer.Models;
using System.Windows;
using System.Windows.Controls;

namespace RX_LabelComposer.Views
{
    /// <summary>
    /// Interaction logic for CountersPage.xaml
    /// </summary>
    public partial class CountersPage : UserControl
    {
        public delegate void FieldSelectedDelegate(string name);
        public FieldSelectedDelegate FieldSelected = null;

        private FieldDefCollection  CounterList;
        private CFieldDef           CounterField = new CFieldDef(){Name="Counter"};
        //--- Constructor ---------------------------------
        public CountersPage()
        {
            InitializeComponent();

            CounterList = new FieldDefCollection();
            CounterList.Add(CounterField);
            DataFieldsGrid.DataSource = CounterList;
        }

        //--- Window_Loaded -------------------------------------------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            //--- Data Context -----------------------------------
            DataContext = this;
        }
        
        //--- Property CounterDef ---------------------------------------
        private CounterDefinition _CounterDef;
        public CounterDefinition CounterDef
        {
            get { return _CounterDef; }
            set 
            { 
                _CounterDef = value; 
                DataContext = value;
            }
        }

        //--- ActiveRecNoChanged --------------------------------------------------------
        public void ActiveRecNoChanged(int recNo)
        {
            _CounterDef.ActiveRecNoChanged(recNo);
            CounterField.Content=_CounterDef.Counter;
        }

        //--- Use_Clicked -----------------------------------------------------
        private void Use_Clicked(object sender, RoutedEventArgs e)
        {
            if (FieldSelected != null) FieldSelected("Counter");
        }
    }
}
