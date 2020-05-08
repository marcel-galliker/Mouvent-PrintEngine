using RX_LabelComposer.Models;
using Infragistics.Windows.DataPresenter;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace RX_LabelComposer.Common
{
    internal class XamDataGridBuilder
    {

        internal class AddingFieldEventArgs : EventArgs
        {
            public AddingFieldEventArgs(UnboundField field)
            {
                Field = field;
            }

            public UnboundField Field { get; private set; }
        }

        XamDataGrid _grid;
        DynamicTable _tableData;
        FieldLayout _fieldLayout;
        Style _cellValuePresenterStyle;
        Style _labelPresenterStyle;

        public XamDataGridBuilder(XamDataGrid grid, DynamicTable table)
        {
            _grid = grid;
            _tableData = table;
        }

        public event EventHandler<AddingFieldEventArgs> AddingField;

        public void BuildGrid()
        {
            _grid.FieldLayouts.Clear();
            _grid.FieldSettings.AllowEdit = false;
            _grid.FieldSettings.CellClickAction = CellClickAction.SelectRecord;
//            _grid.FieldLayoutSettings.AutoFitMode = AutoFitMode.Always;
            _grid.FieldLayoutSettings.AutoGenerateFields = false;
            _grid.FieldLayoutSettings.SelectionTypeRecord = Infragistics.Windows.Controls.SelectionType.Single;
            _fieldLayout = new FieldLayout();
            _fieldLayout.Settings.AutoArrangeCells = AutoArrangeCells.Never;
            _fieldLayout.FieldSettings.CellClickAction = CellClickAction.SelectRecord;
            _cellValuePresenterStyle = (Style)_grid.FindResource("GtCellValuePresenter");
            _labelPresenterStyle = (Style)_grid.FindResource("GtLabelPresenter");

            var colIdx = 0;
            foreach (var header in _tableData.Columns)
            {
                AddField(colIdx, header);
                colIdx ++;
            }

            _grid.FieldLayouts.Add(_fieldLayout);

            if (_tableData != null && _tableData.Rows != null)
            {
                _grid.DataSource = _tableData.Rows;
            }
        }

        private void AddField(int colIdx, DynamicColumn header)
        {
            var field = new UnboundField();
            field.Column = colIdx;
            field.Label = header.Label;
            field.Name = header.Name;

            field.BindingPath = new PropertyPath(string.Format("Cells[{0}]", colIdx));
            field.Settings.CellValuePresenterStyle  = _cellValuePresenterStyle;
            field.Settings.LabelPresenterStyle      = _labelPresenterStyle;
            
            field.Width = FieldLength.Auto;
            if (AddingField != null) 
                AddingField(this, new AddingFieldEventArgs(field));

            _fieldLayout.Fields.Add(field);
        }
    }
}
