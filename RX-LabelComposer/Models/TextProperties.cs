using RX_Common;
using RX_LabelComposer.External;
using RX_LabelComposer.Models.Enums;
using System;

namespace RX_LabelComposer.Models
{
    public class TextProperties : RxBindable
    {
        //--- Constructor -----------------------------------
        public TextProperties(STextBox textBoxS)
        {
            TextBoxStruct = textBoxS;
            /* // commented by samuel
            //--- default values ---
            Font      = "Arial";
            Size      = 12;
            LineSpace = 5;
            */
        }

        //--- TextBoxStruct ---------------------------------
        private STextBox _TextBoxS;
        public STextBox TextBoxStruct
        {
            get { return _TextBoxS; }
            set { SetProperty(ref _TextBoxS, value); }
        }

        //--- FontList --------------------------------------
        public FontList FontList
        {
            get { return new FontList(false); }
        }

        //--- Font ------------------------------------------
        public string Font
        {
            get { return _TextBoxS.font; }
            set { SetProperty(ref _TextBoxS.font, value); }
        }

        //--- Size ------------------------------------------
        public int Size
        {
            get { return _TextBoxS.size; }
            set { SetProperty(ref _TextBoxS.size, value); }
        }

        //--- LineSpace --------------------------------------
        public double LineSpace
        {
            get { return _TextBoxS.lineSpace/1000.0; }
            set { SetProperty(ref _TextBoxS.lineSpace, Convert.ToInt32(value*1000.0)); }
        }

        //--- AlignmentList -----------------------------------
        public AlignmentList AlignmentList
        {
            get { return new AlignmentList(); }
        }

        //--- Alignment -------------------------------------------
        public AlignmentEnum Alignment
        {
            get { return _TextBoxS.alignment; }
            set { SetProperty(ref _TextBoxS.alignment, value); }
        }
    }
}
