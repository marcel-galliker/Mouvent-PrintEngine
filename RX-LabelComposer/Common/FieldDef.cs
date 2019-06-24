using RX_Common;
using System.Collections.ObjectModel;

namespace RX_LabelComposer.Common
{
    public class CFieldDef : RxBindable
    {
        public static int RecLen;


        //--- Property Name -----------------------
        private string _Name;
        public string Name
        {
            get { return _Name; }
            set { SetProperty(ref _Name, value);}
        }
        
        //--- Property Pos -----------------------
        private int _Pos;
	    public int Pos
	    {
		    get { return _Pos;}
		    set { SetProperty(ref _Pos, value);}
	    }

        //--- Property Len -----------------------
        private int _Len;
        public int Len
        {
            get { return _Len; }
            set 
            { 
                if (Pos+value<=RecLen) SetProperty(ref _Len, value);
                else                   SetProperty(ref _Len, RecLen-Pos);
            }
        }

        //--- Property Content -----------------------
        private string _Content;
        public string Content
        {
            get { return _Content; }
            set { SetProperty(ref _Content, value); }
        }
        
	}

    public class FieldDefCollection : ObservableCollection<CFieldDef> { }
}
