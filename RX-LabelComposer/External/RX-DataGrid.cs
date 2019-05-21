using System;
using System.Windows;
using System.Windows.Data;
using Infragistics.Windows.DataPresenter;
using RX_LabelComposer.Common;
using RX_LabelComposer.Models.Enums;
using System.Windows.Controls;

namespace RX_LabelComposer.External
{
    public class RX_DataGrid
    {
        public Action<int> OnActiveRecNoChanged;


        private XamDataGrid _grid;
        private Style _labelPresenterStyle;
        private Style _activeLabelPresenterStyle;
        private Style _cellValuePresenterStyle;
        private Style _activeCellValuePresenterStyle;
        private FieldLayout _fieldLayout = new FieldLayout();
        private int   _activeColumn=-1;

        public RX_DataGrid(XamDataGrid grid, System.Collections.IEnumerable dataSource)
        {
            if (_grid == null)
            {
                _grid = grid;
                _grid.FieldSettings.AllowEdit = false;
                _grid.FieldSettings.CellClickAction = CellClickAction.SelectRecord;
                //  _grid.FieldLayoutSettings.AutoFitMode = AutoFitMode.Always;
                _grid.FieldLayoutSettings.AutoGenerateFields = false;
                _grid.FieldLayoutSettings.HighlightAlternateRecords=true;
                _grid.FieldLayoutSettings.SelectionTypeRecord = Infragistics.Windows.Controls.SelectionType.Single;
                _cellValuePresenterStyle        = (Style)_grid.FindResource("RX_CellValuePresenter");
                _activeCellValuePresenterStyle  = (Style)_grid.FindResource("RX_ActiveCellValuePresenter");
                _labelPresenterStyle            = (Style)_grid.FindResource("RX_LabelPresenter");
                _activeLabelPresenterStyle      = (Style)_grid.FindResource("RX_ActiveLabelPresenter");
                _fieldLayout.Settings.AutoArrangeCells = AutoArrangeCells.Never;
                _fieldLayout.FieldSettings.CellClickAction = CellClickAction.SelectRecord;
                _grid.FieldLayouts.Add(_fieldLayout);
                if (dataSource != null) _grid.DataSource = dataSource;
            }
        }

        //--- addField --------------------------------------------------------------------
        private void addField(int idx)
        {
            var field = new UnboundField();
            field.Column = idx;
            field.BindingPath = new PropertyPath(string.Format("Cells[{0}]", idx));
            field.Settings.CellValuePresenterStyle = _cellValuePresenterStyle;
            field.Settings.LabelPresenterStyle     = _labelPresenterStyle;
            field.Width = FieldLength.Auto;

            _fieldLayout.Fields.Add(field);
        }

        //--- SetFieldCount ------------------------------------------------------------------
        public void SetFieldCount(int count)
        {
            int i = _fieldLayout.Fields.Count;
            while (count >= i) addField(i++);
            while (i > count)
            {
                _fieldLayout.Fields.RemoveAt(count);
                i--;
            }
        }

        //--- SetField --------------------------------------------------------------------
        public void SetField(int idx, string name)
        {
            int i = _fieldLayout.Fields.Count;
            while (idx >= i) addField(i++);
            _fieldLayout.Fields[idx].Name  = name;
            _fieldLayout.Fields[idx].Label = name;
        }

        //--- SetActiveColumn --------------------------------------------------------------------------
        public void SetActiveColumn(int idx)
        {
            if (idx!=_activeColumn)
            {
                if (_activeColumn>=0 && _activeColumn<_fieldLayout.Fields.Count)
                {
                    _fieldLayout.Fields[_activeColumn].Settings.LabelPresenterStyle     = _labelPresenterStyle;
                    _fieldLayout.Fields[_activeColumn].Settings.CellValuePresenterStyle = _cellValuePresenterStyle;
                }
                if (idx>=0 && idx<_fieldLayout.Fields.Count) 
                {
                    _fieldLayout.Fields[idx].Settings.LabelPresenterStyle     = _activeLabelPresenterStyle;
                    _fieldLayout.Fields[idx].Settings.CellValuePresenterStyle = _activeCellValuePresenterStyle;
                }
                _activeColumn = idx;
            }
        }

        //--- Property Height -------------------------------------------------------------------
        public double Height
        {
            get { return _grid.Height; }
            set { _grid.Height = value; }
        }
             
        //--- SetRecordFields -------------------------------------------------------------------
        public void SetRecordFields(FieldDefCollection fields, FileFormatEnum fileFormat)
        {
            SetFieldCount(fields.Count);
            int idx = 0;
            foreach (CFieldDef field in fields)
            {
                _fieldLayout.Fields[idx].Name  = field.Name;
                _fieldLayout.Fields[idx].Label = field.Name;
                _fieldLayout.Fields[idx].Width = null;
                _fieldLayout.Fields[idx].Width = FieldLength.Auto;
                idx++;
            }
            if (fileFormat==FileFormatEnum.Fixed)
            {
                SetField(idx, "Rest");
                _fieldLayout.Fields[idx].Width = null;
                _fieldLayout.Fields[idx].Width = FieldLength.Auto;
                idx++;
            }
        }

        //--- ActiveRecNo --------------------------------------------------------------------------
        public int ActiveRecNo
        {
            get 
            {
                try   { return _grid.ActiveRecord.Index; }
                catch { return 0; }
            }
            set 
            {
                int index = 0;
                try 
                { 
                    index = _grid.ActiveRecord.Index;
                    _grid.Records[index].IsActive = false;
                    _grid.Records[index].IsSelected=false;
                }
                catch{}

                try   { _grid.ActiveRecord = _grid.Records[value];}
                catch { _grid.ActiveRecord = _grid.Records[index];}

                if (OnActiveRecNoChanged != null) OnActiveRecNoChanged(value);
            }
        }
        
    }

    //--- MyConverter -----------------------------------------------------------------------
    public class MyConverter : IMultiValueConverter
    {
        #region IMultiValueConverter Members

        public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if (values != null && values[0] != null)
            {
                Record r = values[0] as Record;
                return (r.VisibleIndex+1).ToString();
            }
            else return null;
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }

        #endregion
    }

}
