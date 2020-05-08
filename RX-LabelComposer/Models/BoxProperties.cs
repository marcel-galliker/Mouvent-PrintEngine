using RX_Common;
using RX_LabelComposer.External;
using RX_LabelComposer.Models.Enums;
using System;
using System.Collections.ObjectModel;
using System.Windows.Controls;

namespace RX_LabelComposer.Common
{
    public class BoxProperties : RxBindable
    {
        static private double factor = 1000.0;

        private SLayoutBox _Box;

        //--- property Delay ----------------------
        private static int _Delay;
        public static int Delay
        {
            set { _Delay = value; }
            get { return _Delay; }
        }

        //--- Property LabelBox ----------------------------
        private LabelBox   _LabelBox;
        public LabelBox LabelBox
        {
            get { return _LabelBox; }
            set { _LabelBox = value; }
        }
        
        //--- Property BoxProperties -------------------------
        public BoxProperties(SLayoutBox def)
        {
            _Box = def;
            if (_Box.hdr.width == 0) _Box.hdr.width = 500;
            if (_Box.hdr.height == 0) _Box.hdr.height = 100;
            _LabelBox = new LabelBox(_Box.hdr.x, _Box.hdr.y, _Box.hdr.width, _Box.hdr.height, this);
        }

        //--- ToUnmanaged -------------------------------
        public void ToUnmanaged(ref SLayoutBox dst)
        {
            dst = _Box;
        }

        private int _No;
        public int No
        {
            get { return _No; }
            set { 
                if (value!=_No) 
                    _No =_LabelBox.No = value; 
            }
        }    

        //--- Property Type -------------------------------------------------
        public BoxTypeEnum Type
        {
            get { return _Box.hdr.boxType; }
            set { SetProperty(ref _Box.hdr.boxType, value); }
        }

        //--- Property BoxTypeList ------------------------------------------------
        public BoxTypeList BoxTypeList
        {
            get { return new BoxTypeList(); }
        }

        //--- Property PosX ------------------------------------------------------
	    public double PosX
	    {
            get { return _Box.hdr.x / factor; }
            set 
            {
                Delay = 2;
                _LabelBox.X = Convert.ToInt32(value * factor);
                SetProperty(ref _Box.hdr.x, _LabelBox.X); 
            }
	    }

        //--- Property PosY -------------------------------------------------------
        public double PosY
        {
            get { return _Box.hdr.y / factor; }
            set 
            {
                Delay = 2;
                _LabelBox.Y = Convert.ToInt32(value * factor);
                SetProperty(ref _Box.hdr.y, _LabelBox.Y); 
            }
        }

        //--- Property Width -------------------------------------------------------
	    public double Width
	    {
            get { return _Box.hdr.width / factor; }
            set 
            {
                _LabelBox.W = Convert.ToInt32(value * factor);
                SetProperty(ref _Box.hdr.width, _LabelBox.W); 
            }
	    }
	
        //--- Property Height --------------------------------------------------------
	    public double Height
	    {
            get { return _Box.hdr.height / factor; }
		    set 
            {
                _LabelBox.H = Convert.ToInt32(value * factor);
                SetProperty(ref _Box.hdr.height, _LabelBox.H);
            }
	    }

        //--- Property OrientationLst ----------------------------------------------------
        public OrientationList OrientationList
        {
            get { return new OrientationList(); }
        }

        //--- Property Orientation ----------------------------------------------------
        public int Orientation
        {
            get { return _Box.hdr.orientation; }
            set { SetProperty(ref _Box.hdr.orientation, value); }
        }
        
        //--- Property Mirror ---------------------------------------------------------
        public int Mirror
	    {
		    get { return _Box.hdr.mirror;}
		    set { SetProperty(ref _Box.hdr.mirror, value);}
	    }

        //--- Property Content --------------------------------------------------------
        public string Content
        {
            get { return _Box.hdr.content; }
            set { SetProperty(ref _Box.hdr.content, value);}
        }

        //--- Property TextBoxStruct --------------------------------------------------
        public STextBox TextBoxStruct
        {
            get { return _Box.text; }
            set { SetProperty(ref _Box.text, value); }
        }

        //--- Property BarCodeStruct ----------------------------------------------------
        public SBarcodeBox BarcodeBoxStruct
        {
            get { return _Box.bc; }
            set { SetProperty(ref _Box.bc, value); }
        }

        //--- Property PropertiesScrollVisible ---------------------------------------------
        private ScrollBarVisibility _PropertiesScrollVisible = ScrollBarVisibility.Hidden;
        public ScrollBarVisibility PropertiesScrollVisible
        {
            get { return _PropertiesScrollVisible; }
            set { SetProperty(ref _PropertiesScrollVisible, value); }
        }
    }

    public class BoxDefCollection : ObservableCollection<BoxProperties> { }
}
