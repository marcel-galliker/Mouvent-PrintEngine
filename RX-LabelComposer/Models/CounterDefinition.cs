using RX_Common;
using RX_LabelComposer.External;
using RX_LabelComposer.Models.Enums;
using System;
using System.Runtime.InteropServices;
using System.Text;

namespace RX_LabelComposer.Models
{
    public class CounterDefinition : RxBindable
    { 
        public static CounterDefinition The_CounterDefinition = new CounterDefinition();
        private SCounterDef _CounterDef, _CounterDefOrg;

        //--- creator ------------------------------------- 
        public CounterDefinition()
        {
            PropertyChanged += CounterDefinition_PropertyChanged;
        }

        //--- CounterDefinition_PropertyChanged -----------------------------------
        private void CounterDefinition_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (!e.PropertyName.Equals("Counter"))
            { 
                RecordNo.The_RecordNo.RecNo=RecordNo.The_RecordNo.RecNo;    // initialies a refresh
            }
        }

        //--- Property Counter ---------------------------------------
        private string _Counter;
        public string Counter
        {
            get { return _Counter; }
            set { SetProperty(ref _Counter, value); }
        }
        

        //--- Property changed ---------------------------------------
        private bool _changed = false;
        public bool Changed
        {
            get { return _changed; }
            set { SetProperty(ref _changed, value); }
        }
        
        //--- Property Elements ---------------------------------------
        public string Elements
        {
            get { return _CounterDef.elements; }
            set { SetProperty(ref _CounterDef.elements, value); }
        }
        
        //--- Property Elements_N ---------------------------------------
        public string Elements_N
        {
            get { return _CounterDef.elements_n; }
            set { SetProperty(ref _CounterDef.elements_n, value); }
        }
        
        //--- Property Elements_A ---------------------------------------
        public string Elements_A
        {
            get { return _CounterDef.elements_a; }
            set { SetProperty(ref _CounterDef.elements_a, value); }
        }

        //--- Property Elements_X ---------------------------------------
        public string Elements_X
        {
            get { return _CounterDef.elements_x; }
            set { SetProperty(ref _CounterDef.elements_x, value); }
        }

        //--- Property Start ---------------------------------------
        public string Start
        {
            get { return _CounterDef.start; }
            set { SetProperty(ref _CounterDef.start, value); }
        }
        
        //--- Property End ---------------------------------------
        public string End
        {
            get { return _CounterDef.end; }
            set { SetProperty(ref _CounterDef.end, value); }
        }

        //--- Property LeadingZeros ---------------------------------------
        public bool LeadingZeros
        {
            get 
            { 
                if (_CounterDef.leadingZeros==0) return false; else return true; 
            }
            set 
            { 
                int val = (value)? 1:0;
                SetProperty(ref _CounterDef.leadingZeros, val); 
            }
        }

        //--- Property Increment ---------------------------------------
        public int Increment
        {
            get { return (int)_CounterDef.increment; }
            set { SetProperty(ref _CounterDef.increment, (IncrementEnum)value); }
        }
        
        //--- ActiveRecNoChanged --------------------------------------------------------
        public void ActiveRecNoChanged(int recNo)
        {
            StringBuilder str = new StringBuilder(64);
            
            RX_Counter.ctr_set_def(ref _CounterDef);
            RX_Counter.ctr_get_counter(RecordNo.The_RecordNo.CounterNo, str);
            _changed = !RX_Counter.ctr_def_is_equal(ref _CounterDef, ref _CounterDefOrg);
            Counter = str.ToString();
        }

        //--- Load ---------------------------------------------------------------------
        public void Load(IntPtr doc)
        {
            _CounterDefOrg.size = Marshal.SizeOf(_CounterDefOrg);
            if (RX_Counter.ctr_load_def(doc, ref _CounterDefOrg) == 1) 
                throw new Exception("Data Structures do not match!");
            _CounterDef =_CounterDefOrg;
            RX_Counter.ctr_set_def(ref _CounterDef);
            Changed = false;
        }

        //--- Save -------------------------------------------------------------
        public void Save(IntPtr doc)
        {
            RX_Counter.ctr_save_def(doc, ref _CounterDef);
            _CounterDefOrg = _CounterDef;
            Changed = false;
        }

        //--- IncrementList -------------------------------
        public IncrementList IncrementList
        {
            get { return new IncrementList();}
        }

    }

}
