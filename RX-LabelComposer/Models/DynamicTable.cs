using RX_Common;
using System.Collections.ObjectModel;

namespace RX_LabelComposer.Models
{
    public class DynamicTable : RxBindable
    {
        public DynamicTable()
        {
            Columns = new ObservableCollection<DynamicColumn>();
            Rows = new ObservableCollection<DynamicRow>();
        }
        public ObservableCollection<DynamicColumn> Columns { get; set; }
        public ObservableCollection<DynamicRow> Rows { get; set; }

        public void FillWithDemoData()
        {
            Columns.Add(new DynamicColumn { Name = "Col0", Label = "Col0" });
            Columns.Add(new DynamicColumn { Name = "Col1", Label = "Col1" });
            Columns.Add(new DynamicColumn { Name = "Col2", Label = "Col2" });

            DynamicRow row = new DynamicRow();
            row.Cells.Add(new DynamicCell { CellContent = "R0C0" });
            row.Cells.Add(new DynamicCell { CellContent = "R0C1" });
            row.Cells.Add(new DynamicCell { CellContent = "R0C2" });
            Rows.Add(row);

            row = new DynamicRow();
            row.Cells.Add(new DynamicCell { CellContent = "R1C0" });
            row.Cells.Add(new DynamicCell { CellContent = "R1C1" });
            row.Cells.Add(new DynamicCell { CellContent = "R1C2" });
            Rows.Add(row);
        }
    }

    public class DynamicColumn : RxBindable
    {
        private string _name;

        public string Name
        {
            get { return _name; }
            set { SetProperty(ref _name, value); }
        }
        private string _label;

        public string Label
        {
            get { return _label; }
            set { SetProperty(ref _label, value); }
        }
    }

    public class DynamicCell : RxBindable
    {
        private object _cellContent;
        public object CellContent
        {
            get { return _cellContent; }
            set { SetProperty(ref _cellContent, value); }
        }
    }

    public class DynamicRow : RxBindable
    {
        public DynamicRow()
        {
            Cells = new ObservableCollection<DynamicCell>();
        }
        public ObservableCollection<DynamicCell> Cells { get; set; }
    }
}
