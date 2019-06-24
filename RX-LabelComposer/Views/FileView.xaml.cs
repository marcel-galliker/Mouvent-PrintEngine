using Infragistics.Windows.DataPresenter;
using Microsoft.Win32;
using RX_Common;
using RX_LabelComposer.Common;
using RX_LabelComposer.External;
using RX_LabelComposer.Models;
using RX_LabelComposer.Models.Enums;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace RX_LabelComposer.Views
{
    /// <summary>
    /// Interaction logic for FileView.xaml
    /// </summary>
    public partial class FileView : Page
    {
        private RX_DataGrid _DataGrid;

        //---Constructor -------------------------------------------------
        public FileView()
        {
            InitializeComponent();
        }

        //--- Page_Loaded --------------------------------------
        private void Page_Loaded(object sender, System.EventArgs e)
        {
            //--- Data Context -----------------------------------
            DataContext                 = this;
            //Definitions.DataContext     = FileDef;

            //--- init controls -----------------------------------------------
            if (_DataGrid == null) _DataGrid = new RX_DataGrid(DataRecords, FileDef.GridData.Rows);

            //--- callback -------------------------------------
            FileDef.FieldNameChanged   += OnFieldNameChanged;
            RecordNo.The_RecordNo.OnRecNoChanged += OnRecNoChanged;

            //--- fields ------------------------------------
            OnFieldNameChanged(-1);
            _DataGrid.ActiveRecNo=FileDef.ActiveRecNo;
            if (RxGlobals.Settings.DataFontSize==null)
                RxGlobals.Settings.DataFontSize = (double)Resources["FontSize"];
            DataRecords.FontSize = (double)RxGlobals.Settings.DataFontSize;
        }

        //--- OnRecNoChanged -----------------------------------------
        private void OnRecNoChanged(int recNo)
        {
            if (recNo!=_DataGrid.ActiveRecNo)
                _DataGrid.ActiveRecNo = recNo;
        }

        //--- Page_Unloaded --------------------------------------
        private void Page_Unloaded(object sender, RoutedEventArgs e)
        {
            FileDef.FieldNameChanged   -= OnFieldNameChanged;
        }

        //--- Page_SizeChanged ----------------------------------------
        private void Page_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            FileDefBox.Height = e.NewSize.Height - MainGrid.RowDefinitions[2].ActualHeight;
        }

        //--- GroupBox_SizeChanged ------------------------------------
        private void GroupBox_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (_DataGrid != null) _DataGrid.Height = e.NewSize.Height - 50;
        }

        #region properties

        //--- Property FileDef ----------------------------------------------------
        private FileDefinition _FileDef;
        public FileDefinition FileDef
        {
            get { return _FileDef; }
            set { _FileDef = value; }
        }

        //--- Property JobFilename ------------------------------------------------
        private string _JobFilename;
        public string JobFilename
        {
            get { return _JobFilename; }
            set { _JobFilename = value; }
        }
        
        #endregion

        //--- Filename_Click -------------------------------------------------------
        private void Filename_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();

            // dlg.InitialDirectory = "c:\\";
            dlg.Filter = "txt files (*.txt)|*.txt|All files (*.*)|*.*";
            dlg.FilterIndex = 1;
            dlg.RestoreDirectory = true;

            if (dlg.ShowDialog() == true)
                FileDef.DataFilename = dlg.FileName;
        }

        //--- OnFieldNameChanged ---------------------------------------------------
        private void OnFieldNameChanged(int idx)
        {
            if (idx >= 0 && idx<FileDef.FieldList.Count) _DataGrid.SetField(idx, FileDef.FieldList[idx].Name);
            else
            {
                _DataGrid.SetRecordFields(FileDef.FieldList, FileDef.FileFormat);
            //    _DataGrid.ActiveRecNo = 0;
            }
        }

        //--- Filed Toolbar commands -------------------------------------------
        private void FieldInsBefore_Clicked(object sender, RoutedEventArgs e)
        {
            Record r = FieldDefGrid.ActiveRecord;
            int index=0;
            if (r != null) 
            {
                index = r.Index;
                r.IsActive = false;
            }
            FileDef.FieldListInsBefore(index);
            if (index>0) FieldDefGrid.Records[index - 1].IsActive = true;
            _DataGrid.SetRecordFields(FileDef.FieldList, FileDef.FileFormat);
        }

        //--- FieldInsAfter_Clicked --------------------------------------------
        private void FieldInsAfter_Clicked(object sender, RoutedEventArgs e)
        {
            Record currRecord = FieldDefGrid.ActiveRecord;
            int currIndex = (currRecord == null) ? 0 : currRecord.Index;

            if (FieldDefGrid.Records.Count > 0)
            {
                int newIndex = currIndex + 1;
                FieldDefGrid.Records[currIndex].IsActive = false;
                FileDef.FieldListInsAfter(currIndex);
                FieldDefGrid.Records[newIndex].IsActive = true;
            }
            else
            {
                FileDef.FieldListInsBefore(currIndex);
            }

            // add a new field in the list and make it active.
            _DataGrid.SetRecordFields(FileDef.FieldList, FileDef.FileFormat);
        }

       //--- FieldRemove_Clicked ----------------------------------------------
       private void FieldRemove_Clicked(object sender, RoutedEventArgs e)
        {
            Record r = FieldDefGrid.ActiveRecord;

            if (r != null)
            {
                string str = string.Format("Delete the field >>{0}<<", FileDef.FieldList[r.Index].Name);
                if (System.Windows.MessageBox.Show(str, "Delete", MessageBoxButton.YesNo, MessageBoxImage.Question, MessageBoxResult.No) == MessageBoxResult.Yes)
                {
                    FileDef.FieldListRemove(r.Index);
                    _DataGrid.SetRecordFields(FileDef.FieldList, FileDef.FileFormat);
                }
            }
        }

        //--- OnFieldsKeyDown ---------------------------------------------------
        private void OnFieldsKeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            Record r = FieldDefGrid.ActiveRecord;
            if (r!=null && FieldDefGrid.ActiveCell!=null)
            {
                int column = FieldDefGrid.ActiveCell.Field.ActualPosition.Column;
                switch (RxKeys.ValueInt(e))
                {
                    case 1:  FileDef.FieldListInc(r.Index, FieldDefGrid.ActiveCell.Field.ActualPosition.Column); break;
                    case -1: DataRecords.InvalidateMeasure(); 
                             DataRecords.FieldLayouts[0].Fields[r.Index].Width = null;
                             DataRecords.FieldLayouts[0].Fields[r.Index].Width = FieldLength.Auto;
                             FileDef.FieldListDec(r.Index, column);
                             DataRecords.UpdateLayout();
                             break;
                }
            }
        }

        //--- Header_KeyDown -----------------------------------------------------------
        private void Header_KeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            int val = FileDef.Header+RxKeys.ValueInt(e);
            if (val>=0) FileDef.Header = val;
        }

        //--- ReordLen_KeyDown --------------------------------------------------------------
        private void ReordLen_KeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            int val = FileDef.ReordLen + RxKeys.ValueInt(e);
            if (val >= 0) FileDef.ReordLen = val;
        }

        //--- DataRecords_RecordActivated ----------------------------------------------------
        public void DataRecords_RecordActivated(object sender, Infragistics.Windows.DataPresenter.Events.RecordActivatedEventArgs e)
        {
            FileDef.RecordActivated(e.Record.Index);
            RecordNo.The_RecordNo.RecNo = e.Record.Index;
        }

        //--- Analyse_Click ------------------------------------------------------
        private void Analyse_Click(object sender, RoutedEventArgs e)
        {
            _FileDef.Analyse();
            _DataGrid.ActiveRecNo = 0;
        }

        //--- FieldDefGrid_EditModeStarted --------------------------------------------------
        private string FieldVal;  
        private void FieldDefGrid_EditModeStarted(object sender, Infragistics.Windows.DataPresenter.Events.EditModeStartedEventArgs e)
        {
            FieldVal = e.Cell.Value.ToString();
        }

        //--- FieldDefGrid_EditModeEnded ----------------------------------------------------------
        private void FieldDefGrid_EditModeEnding(
            object                                                              sender,
            Infragistics.Windows.DataPresenter.Events.EditModeEndingEventArgs   e)
        {
            if (e.Cell.Field.Name != "Name")
                return;

            string value = e.Editor.Text;
            if (string.IsNullOrWhiteSpace(value))
            {
                e.AcceptChanges = false;
                MessageBox.Show("Data Validation Failed. Field name must be a valid string.");
                e.Cancel = true;
            }
        }

        //--- FieldDefGrid_EditModeEnded -----------------------------------------------------
        private void FieldDefGrid_EditModeEnded(object sender, Infragistics.Windows.DataPresenter.Events.EditModeEndedEventArgs e)
        {
            if (!FieldVal.Equals(e.Cell.Value.ToString()))
            {
                Record r = e.Cell.Record;
                FileDef.FieldListChanged(r.Index, e.Cell.Field.Index);
            }
        }

        //--- DataRecords_PreviewHeaderMouseLeftButtonDown --------------------------------------------------
        private void DataRecords_PreviewHeaderMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            //--- do nothing ! -----
            e.Handled=true;
        }

        private void FileFormat_Changed(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            Visibility visible = (_FileDef.FileFormat==FileFormatEnum.Variable) ? Visibility.Collapsed : Visibility.Visible;
            //FieldsListLayout.Fields[1].Visibility = visible; // commented by samuel
            //FieldsListLayout.Fields[2].Visibility = visible; // commented by samuel
        }

        //--- FieldDefGrid_RecordActivated ----------------------------------------------------------------
        private void FieldDefGrid_RecordActivated(object sender, Infragistics.Windows.DataPresenter.Events.RecordActivatedEventArgs e)
        {
            _DataGrid.SetActiveColumn(e.Record.Index);
        }

        //--- FontSizePlus_Clicked -------------------------------------------
        private void FontSizePlus_Clicked(object sender, RoutedEventArgs e)
        {
            double max=(double)Resources["FontSize"];
            double size = DataRecords.FontSize+1;
            if (size>max) size=max;
            if (size!=DataRecords.FontSize)
            {
                RxGlobals.Settings.DataFontSize = DataRecords.FontSize = size;
            }
        }

        //--- FontSizeMinus_Clicked -------------------------------------------
        private void FontSizeMinus_Clicked(object sender, RoutedEventArgs e)
        {
            double min=6;
            double size=DataRecords.FontSize-1;
            if (size<min) size=min;
            if (size!=DataRecords.FontSize)
            {
                RxGlobals.Settings.DataFontSize = DataRecords.FontSize = size; 
            }
        }
    }
}
