using RX_Common;
using RX_LabelComposer.Common;
using RX_LabelComposer.External;
using RX_LabelComposer.Models.Enums;
using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows;

namespace RX_LabelComposer.Models
{
    public class FileDefinition : RxBindable
    {
        public static FileDefinition The_FileDefinition = new FileDefinition();
               
        private SFileDef _FileDef = new SFileDef();
        private int RecordCnt = 50;
        private int RecordsUsed;
        
        public FileDefinition()
        {
            // Clear();
            _FileDef.size = Marshal.SizeOf(_FileDef);
            GridData = new DynamicTable();

            //--- initial table ------------------
            GridData.Columns.Add(new DynamicColumn { Label = "" });
            for (int i = 0; i < RecordCnt; i++)
            {
                GridData.Rows.Add(new DynamicRow());
            }

            _FieldList = new FieldDefCollection();

            // commented by samuel de santis
            // I see no reason to add a fielddef by default (and lead to errors).
            //_FieldList.Add(new CFieldDef()); 
        }

        //--- Property Changed -----------------------------------------------
        private bool _Changed = false;
        public bool Changed
        {
            get { return _Changed; }
            private set { _Changed = value; }
        }
    
        //--- Property DataFilename ----------------------------------------------
        public string DataFilename
        {
            get { return _FileDef.dataFile; }
            set 
            { 
                if (SetProperty(ref _FileDef.dataFile, value)) 
                {
                    RecordNo.The_RecordNo.RecNo = 0;
                    UpdateGridData(true);
                }
            }
        }

        //--- Property CodePageList --------------------------------------------------------
        public CodePageList CodePageList
        {
            get { return new CodePageList(); }
        }

        //--- Property CodePage --------------------------------------------------------
        public int CodePage
        {
            get { return _FileDef.codePage; }
            set { if (SetProperty(ref _FileDef.codePage, value)) 
                UpdateGridData(true);}
        }

        //--- Property Header --------------------------------------------------------
        public int Header
        {
            get { return _FileDef.header; }
            set { if (SetProperty(ref _FileDef.header, value)) UpdateGridData(true);}
        }

        //--- Property FileFotmatList --------------------------------------------------------
        public FileFormatList FileFormatList
        {
            get { return new FileFormatList(); }
        }

        //--- Property FileFormat --------------------------------------------------------
        public FileFormatEnum FileFormat
        {
            get { return _FileDef.fileFormat; }
            set
            {
                if (SetProperty(ref _FileDef.fileFormat, value))
                {
                    switch (value)
                    {
                        case FileFormatEnum.Fixed: 
                            ShowVarFormat = Visibility.Collapsed;
                            ShowFixFormat = Visibility.Visible;
                            break;
                        case FileFormatEnum.Variable:
                            ShowVarFormat = Visibility.Visible;
                            ShowFixFormat = Visibility.Collapsed;
                            break;
                    }
                    UpdateGridData(true);
                }
            }
        }

        //--- Property FieldSeparatorList --------------------------------------------------------
        public FieldSeparatorList FieldSeparatorList
        {
            get { return new FieldSeparatorList(); }
        }

        //--- Property Filter ---------------------------------------
        private bool _Filter;
        public bool Filter
        {
            get { return _Filter; }
            set 
            { 
                if (SetProperty(ref _Filter, value))
                {
                    _FileDef.filter = value?1:0;
                    UpdateGridData(true);
                }
            }
        }
 
        //--- Property FieldSeparator --------------------------------------------------------
        public FieldSeparatorEnum FieldSeparator
        {
            get { return (FieldSeparatorEnum)_FileDef.fieldSep; }
            set { if(SetProperty(ref _FileDef.fieldSep, value)) UpdateGridData(true); }
        }

        //--- Property ReordLen --------------------------------------------------------
        public int ReordLen
        {
            get { return _FileDef.recLen; }
            set 
            { 
                if (SetProperty(ref _FileDef.recLen, value))
                {
                    CFieldDef.RecLen = value;
                    UpdateGridData(true); 
                }
            }
        }

        //--- Property FieldList --------------------------------------------------------
        private FieldDefCollection _FieldList;
        public FieldDefCollection FieldList
        {
            get { return _FieldList; }
            set { SetProperty(ref _FieldList, value); }
        }

        //--- Property ShowVarFormat --------------------------------------------------------
        private Visibility _ShowVarFormat;
        public Visibility ShowVarFormat
        {
            get { return _ShowVarFormat; }
            set { SetProperty(ref _ShowVarFormat, value); }
        }

        //--- Property ShowFixFormat --------------------------------------------------------
        private Visibility _ShowFixFormat;
        public Visibility ShowFixFormat
        {
            get { return _ShowFixFormat; }
            set { SetProperty(ref _ShowFixFormat, value); }
        }       

        //--- SetProperties --------------------------------------------------------
        private void SetProperties(SFileDef fileDef)
        {
            //--- launch the change events ------------------
            DataFilename    = fileDef.dataFile;
            Header          = fileDef.header;
            CodePage        = fileDef.codePage;
            FileFormat      = fileDef.fileFormat;
            ReordLen        = fileDef.recLen;
            FieldSeparator  = fileDef.fieldSep;
            Filter          = (fileDef.filter!=0);

            FieldDefCollection list = new FieldDefCollection();
            foreach (SFieldDef field in fileDef.field)
            {
                if (field.name == "") break;
                list.Add(new CFieldDef() { Name = field.name, Pos = field.pos, Len = field.len });
            }
            FieldList = list;

            UpdateGridData();
        }

        //--- FieldList_Cs2Cpp --------------------------------------------------------------------
        private void FieldList_Cs2Cpp(ref SFileDef fileDef)
        {
            int i = 0;

            if (fileDef.field == null) fileDef.field = new SFieldDef[64];
            foreach (SFieldDef field in fileDef.field)
            {
                fileDef.field[i++].name = "";
            }
            i = 0;
            foreach (CFieldDef field in FieldList)
            {
                fileDef.field[i] = new SFieldDef();
                fileDef.field[i].name = field.Name;
                fileDef.field[i].pos  = field.Pos;
                fileDef.field[i].len  = field.Len;
                i++;
            }
        }

        //--- UpdateGridData -------------------------------------------------------------------
        private void UpdateGridData(bool force=false)
        {
            //--- fill data records ------------------------------------------------
            int i, ret, len, pos;
            byte[] buffer = new Byte[RX_Dat.dat_max_record_size()];

            FieldList_Cs2Cpp(ref _FileDef);
//            Changed = true;

            RX_Dat.dat_set_file_def(ref _FileDef);
            RX_Dat.dat_seek(0);
            RecordsUsed = 0;
            
            for (int recNo = 0; recNo < RecordCnt; recNo++)
            {
                ret=RX_Dat.dat_read_next_record();
                if (recNo >= GridData.Rows.Count) GridData.Rows.Add(new DynamicRow());
                DynamicRow row = GridData.Rows[recNo];
                i=0;
                pos=0;
                if (ret!=0)
                { 
                    RecordsUsed++;
                    for (i = 0; i < FieldList.Count; i++)
                    {
                        if (i >= row.Cells.Count) row.Cells.Add(new DynamicCell());
                        len = RX_Dat.dat_get_field(i, buffer, buffer.Length);
                        pos = _FileDef.field[i].pos+_FileDef.field[i].len;
                        row.Cells[i].CellContent = Encoding.Unicode.GetString(buffer, 0, len);
                    }
                    if (_FileDef.fileFormat==FileFormatEnum.Fixed)
                    {
                        if (i >= row.Cells.Count) row.Cells.Add(new DynamicCell());
                        len = _FileDef.recLen-pos;
                        if (len>0)  
                        {
                            len=RX_Dat.dat_get_buffer(pos, len, buffer);
                            // row.Cells[i].CellContent = string.Format("pos={0} len={1}", pos.ToString(), _FileDef.recLen-pos);
                            row.Cells[i].CellContent = Encoding.Unicode.GetString(buffer, 0, len);
                        }
                        else row.Cells[i].CellContent = null;
                    }
                }

              //  while (i < FieldList.Count && i<row.Cells.Count)
              //      row.Cells.RemoveAt(i++);                
            }
            while (GridData.Rows.Count>RecordsUsed) GridData.Rows.RemoveAt(RecordsUsed);
            RecordActivated(ActiveRecNo);
        }

        //--- Property ActiveRecNo ---------------------------------------
        private int _ActiveRecNo;
        public int ActiveRecNo
        {
            get { return _ActiveRecNo; }
            set { SetProperty(ref _ActiveRecNo, value); }
        }
        

        //--- RecordActivated ------------------------------------------------------------
        public void RecordActivated(int index)
        {
            if (index<RecordsUsed)
            {
                ActiveRecNo = index;
                for (int i = 0; i < FieldList.Count; i++)
                {
                    FieldList[i].Content = (i<GridData.Rows[index].Cells.Count)? GridData.Rows[index].Cells[i].CellContent.ToString():null;
                }
            }
        }

        public delegate void FieldNameChangedDelegate(int col);
        public FieldNameChangedDelegate FieldNameChanged = null;

        //--- FieldListInsBefore -------------------------------------------------------
        public void FieldListInsBefore(int index)
        {
            FieldList.Insert(index, new CFieldDef() { Name = "new" });
            UpdateGridData();
            if (FieldNameChanged != null) FieldNameChanged(index);
        }

        //--- FieldListInsAfter ---------------------------------------------------------
        public void FieldListInsAfter(int index)
        {
            FieldList.Insert(index + 1, new CFieldDef() { Name = "new" });
            if (FileFormat==FileFormatEnum.Fixed)
            {
                if (index>=0) FieldList[index+1].Pos= FieldList[index].Pos+FieldList[index].Len+1;
                FieldList[index+1].Len = ReordLen - FieldList[index+1].Pos;
            }
            UpdateGridData();
            if (FieldNameChanged != null) FieldNameChanged(index);
        }

        //--- FieldListRemove ----------------------------------------------------------
        public void FieldListRemove(int index)
        {
            FieldList.RemoveAt(index);
            UpdateGridData();
            if (FieldNameChanged != null) FieldNameChanged(index);
        }

        //--- FieldListInc --------------------------------------------------------------
        public void FieldListInc(int index, int col)
        {
            if (FieldList[index].Pos+FieldList[index].Len<_FileDef.recLen)
            {
                switch (col)
                {
                    case 1: FieldList[index].Pos++; break;
                    case 2: FieldList[index].Len++; break;
                }
                UpdateGridData();
            }
        }

        //--- FieldListDec ---------------------------------------------------------------
        public void FieldListDec(int index, int col)
        {
            switch (col)
            {
                case 1: if (FieldList[index].Pos <= 0) return;
                        FieldList[index].Pos--; break;
                case 2: if (FieldList[index].Len <= 0) return;
                        FieldList[index].Len--;
                        break;
            }
            UpdateGridData();
        }

        //--- FieldListChanged ------------------------------------------------------------
        public void FieldListChanged(int index, int coloumn)
        {
            if (coloumn == 0 && FieldNameChanged != null) FieldNameChanged(index);
            UpdateGridData();
        }

        //--- Property GridData -----------------------------------------------------------
        private DynamicTable _GridData;
        public DynamicTable GridData
        {
            get { return _GridData; }
            set { _GridData = value; }
        }

        //-- Load ---------------------------------------------------------------------
        public void Load(IntPtr doc)
        {
            SFileDef fileDef = new SFileDef();
            fileDef.size = Marshal.SizeOf(fileDef);
//            if (RX_Dat.dat_load_file_def(doc, System.IO.Path.GetTempPath(), ref fileDef)==0)
            if (RX_Dat.dat_load_file_def(doc, RxGlobals.Dir, ref fileDef)==0)
            {
                SetProperties(fileDef);
                if (FieldNameChanged != null) FieldNameChanged(-1);
            }
            Changed = false;
        }

        //---  Save -----------------------------------------------------------
        public void Save(StringBuilder path, IntPtr doc)
        {
            FieldList_Cs2Cpp(ref _FileDef);
            RX_Dat.dat_save_file_def(doc, System.IO.Path.GetTempPath(), ref _FileDef, path.ToString());
            Changed = false;
        }

        //--- Analyse ---------------------------------------------------------
        public void Analyse()
        {
            if(System.Windows.MessageBox.Show("All Settings may be changed. Continue?", "Analyse", MessageBoxButton.YesNo, MessageBoxImage.None, MessageBoxResult.No)==MessageBoxResult.Yes)
            {
                SFileDef fileDef = new SFileDef();
                fileDef.size = Marshal.SizeOf(fileDef);
                fileDef.dataFile = _FileDef.dataFile;
                RX_Dat.dat_analyse_file(ref fileDef);
                SetProperties(fileDef);
                if (FieldNameChanged != null) FieldNameChanged(-1);
                RecordNo.The_RecordNo.RecNo = 0;           
            }
        }

        //--- RecToFieldNames -----------------------------------------------------
        public void RecToFieldNames()
        {
            if(System.Windows.MessageBox.Show("Fieldnames will be changed. Continue?", "First Record as Fieldnames", MessageBoxButton.YesNo, MessageBoxImage.None, MessageBoxResult.No)==MessageBoxResult.Yes)
            {
                int recLen, i, len;
                byte[] buffer = new Byte[256];

                RX_Dat.dat_seek(0);
                recLen =RX_Dat.dat_read_next_record(); 
                if (recLen!=0)
                {
                    for (i = 0; i < FieldList.Count; i++)
                    {
                        len = RX_Dat.dat_get_field(i, buffer, buffer.Length);
                        FieldList[i].Name = Encoding.Unicode.GetString(buffer, 0, len);
                    }
                    Header += recLen;
                }
            }
        }
    }
}
