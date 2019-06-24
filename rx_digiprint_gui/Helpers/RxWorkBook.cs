using RX_Common;
using SmartXLS;
using System.Windows.Media;

namespace RX_DigiPrint.Helpers
{
    public class RxWorkBook : WorkBook
    {
        private int _Sheet = -1;

        public string FileName;

        //--- AddSheet ----------------------------------------------------------------------
        public void AddSheet(string name)
        {
            if (_Sheet<0) _Sheet=0;
            else
            {
                NumSheets++;
                _Sheet++;
            }
            Sheet = _Sheet;
            setSheetName(Sheet, name);
        }

        //--- setText --------------------------------------
        public void setText(int row, int col, object obj)
        {
            if (obj!=null) base.setText(row, col, obj.ToString());
        }

        //--- getFieldStyle ------------------------------------
        private RangeStyle getFieldStyle(int row, int col)
        {
            char column;
            string field;
            column = 'A';
            column+=(char)col;          
            field  = string.Format("{0}{1}", column, row+1);
            setSelection(field);
            return getRangeStyle();
        }

        //--- SetFieldColor ---------------------------------------------------
        public void SetFieldColor(int row, int col, Color fg, Color bg)
        {
            RangeStyle rangeStyle = getFieldStyle(row, col);
            rangeStyle.Pattern=1;
            rangeStyle.PatternFG = (int)Rx.ToArgb(bg);
            rangeStyle.FontColor = (int)Rx.ToArgb(fg);
            setRangeStyle(rangeStyle);
        }

        //--- SetRowColor ---------------------------------------------------
        public void SetRowColor(int row, Color fg, Color bg)
        {
            int col;
            for (col=0; col<=LastCol; col++)
            {
                RangeStyle rangeStyle = getFieldStyle(row, col);
                rangeStyle.Pattern=1;
                rangeStyle.PatternFG = (int)Rx.ToArgb(bg);
                rangeStyle.FontColor = (int)Rx.ToArgb(fg);
                setRangeStyle(rangeStyle);
            }
        }

        //--- HeaderRow -------------------------------------
        public void HeaderRow(int row)
        {
            int col;
            RangeStyle style;
            for (col=0; col<=LastCol; col++)
            {
                SetFieldColor(row, col, Colors.Black, Colors.LightGray);
                style = getFieldStyle(row, col);
                style.FontBold = true;
                setRangeStyle(style);
            }
        }

        //--- SizeColumns -----------------------------
        public void SizeColumns()
        {
            int col;
            Sheet=0;
            while(true)
            {
                for (col=0; col<=LastCol; col++) setColWidthAutoSize(col,   true);
                if (Sheet+1==NumSheets) break;
                Sheet++;
            };
        }
    }
}
