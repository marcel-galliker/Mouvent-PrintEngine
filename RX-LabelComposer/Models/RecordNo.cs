using RX_Common;
using RX_LabelComposer.External;
using RX_LabelComposer.ViewModels;
using System;
using System.Windows.Input;

namespace RX_LabelComposer.Models
{
    public class RecordNo : RxBindable, IRecordQuickNavigation
    {
        public static RecordNo The_RecordNo = new RecordNo();


        //--- Delegate ------------------------------------
        public Action<int> OnRecNoChanged;  

        //--- Creator -----------------------------------
        public RecordNo()
        {
            this.ShowFirstRecordCommand    = new RxCommand(DoFirst);
            this.ShowPreviousRecordCommand = new RxCommand(DoPrev);
            this.ShowNextRecordCommand     = new RxCommand(DoNext);
        }

        //--- IncStep -----------------------------------------------
        private int IncStep()
        {
            if (FileDefinition.The_FileDefinition.DataFilename == null || FileDefinition.The_FileDefinition.DataFilename.Equals(""))
            {
                if (CounterDefinition.The_CounterDefinition.Increment==(int)IncrementEnum.INC_perRow) return 1;
                else return LayoutDefinition.The_LayoutDef.Columns;
            }
            if (LayoutDefinition.The_LayoutDef.Columns>0) return LayoutDefinition.The_LayoutDef.Columns;
            else return 1;
        }

        //--- RecNoChanged ---------------------
        public void RecNoChanged(int recNo)
        {
            RecNo=recNo;
        }

        //--- Property RecNo ---------------------------------------
        private int _RecNo;
        public int RecNo
        {
            get { return _RecNo; }
            set 
            { 
              //  int step = IncStep();
              //  int no = (int)value;
              //  SetProperty(ref _RecNo, (no/step)*step);
                SetProperty(ref _RecNo, value);
                if (OnRecNoChanged!=null) OnRecNoChanged(_RecNo);
                RecNoStr = (_RecNo+1).ToString();
            }
        }

        //--- Property RecNoStr ---------------------------------------
        private string _RecNoStr;
        public string RecNoStr
        {
            get { return _RecNoStr; }
            set { SetProperty(ref _RecNoStr, value); }
        }
     
        
        //--- CounterNo ----------------------------------------- 
        public int CounterNo
        {
            get 
            {
                if (FileDefinition.The_FileDefinition.DataFilename == null || FileDefinition.The_FileDefinition.DataFilename.Equals(""))
                return RecNo;
                
                int step = IncStep();
                if (step>2) return RecNo/step;
                else return RecNo;
            }
        }

        //--- Command First --------------------------------
        void DoFirst(object parameter)
        {
            RecNo=0;
        }

        //--- Command First --------------------------------
        void DoNext(object parameter)
        {
            int step = IncStep();
            RecNo= (_RecNo / step + 1)*step;
        }

        //--- Command First --------------------------------
        void DoPrev(object parameter)
        {
            int step = IncStep();           
            if (RecNo>step) RecNo=(RecNo / step - 1)*step;
            else RecNo=0;
        }

        #region Record Quick Navigation

        //--- Commands ----------------------------------------------------------------------------
        public ICommand ShowFirstRecordCommand { get; private set; }
        public ICommand ShowNextRecordCommand { get; private set; }
        public ICommand ShowPreviousRecordCommand { get; private set; }

        #endregion
    }
}
