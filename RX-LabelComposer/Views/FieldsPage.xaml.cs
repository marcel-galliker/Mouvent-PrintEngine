using Infragistics.Windows.DataPresenter;
using RX_LabelComposer.Common;
using RX_LabelComposer.Models;
using System.Windows.Controls;
using System.Windows.Input;

namespace RX_LabelComposer.Views
{
    /// <summary>
    /// Interaction logic for FieldsWnd.xaml
    /// </summary>
    public partial class FieldsPage : Page
    {
        public delegate void FieldSelectedDelegate(string name);
        public FieldSelectedDelegate FieldSelected = null;

        // SAMUEL
        //public FieldDefCollection FieldList => this.FileDef?.FieldList;

        //--- Constructor ---------------------------------
        public FieldsPage()
        {
            InitializeComponent();
            DataContext = this;
        }

        //--- Property FileDef -------------------------------------
        private FileDefinition _FileDef;
        public FileDefinition FileDef
        {
            get { return _FileDef; }
            set 
            {
                _FileDef = value;
                DataFieldsGrid.DataSource = _FileDef.FieldList;
                _FileDef.RecordActivated(0);
            }
        }

        //--- DataFieldsGrid_MouseDoubleClick -------------------------------------
        private void DataFieldsGrid_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            Record r = DataFieldsGrid.ActiveRecord;
            if (r != null)
            {
                string name = _FileDef.FieldList[r.Index].Name;
                if (FieldSelected != null)
                    FieldSelected(name);
            }
        }

    }
}
