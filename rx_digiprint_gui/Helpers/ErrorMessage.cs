using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System.Text;

// using Microsoft.Office.Interop.Excel;
// using Excel = Microsoft.Office.Interop.Excel;


namespace RX_DigiPrint.Helpers
{
    class ErrorMessage
    {
        // private static object[,] _ErrTexts=null;

        //--- init -------------------------------------------------------------------------
        private static void init()
        {
            /*
            if (_ErrTexts == null)
            {
                Directory.GetCurrentDirectory();
              
                Language = "english";
                string filepath = Directory.GetCurrentDirectory() + "\\Language.xlsx";

                Microsoft.Office.Interop.Excel.Application _Excel = new Microsoft.Office.Interop.Excel.Application();
                Excel.Workbook   _Workbook = null;
                Excel.Worksheet _WorkSheet = null;
                //--- http://www.abouchleih.com/index.php/2013/07/26/auslesen-einer-excel-datei-in-c/
                _Excel.Visible = false;
                _Excel.ScreenUpdating = false;
                _Excel.DisplayAlerts = false;
                _Workbook = _Excel.Workbooks.Open(filepath, Missing.Value, false);
                _WorkSheet = (Excel.Worksheet)_Workbook.Worksheets.get_Item(1);
                string alphabet = "ABCDEFGHIJKLMNOPQRSTUVQXYZ";
                int rowCount = _WorkSheet.UsedRange.Rows.Count;
                int colCount = _WorkSheet.UsedRange.Columns.Count;
                char lastColChar = alphabet[colCount];
                Range range = _WorkSheet.get_Range("A1", lastColChar.ToString() + rowCount.ToString());
                _ErrTexts = (object[,])range.Value2;

                //--- clean up ---
                range = null;
                Marshal.FinalReleaseComObject(_WorkSheet);
                _Excel.DisplayAlerts = false;
                _WorkSheet = null;
                GC.Collect();
                GC.WaitForPendingFinalizers();

                _Workbook.Close(false, Missing.Value, Missing.Value);
                Marshal.FinalReleaseComObject(_Workbook);
                _Workbook = null;

                _Excel.Quit();
                Marshal.FinalReleaseComObject(_Excel);
                _Excel = null;
            }
            */
        }

        //--- TranslateFormatStr --------------------------------------------
        /*
        private static string TranslateFormatStr(RX_DigiPrint.Services.TcpIp.SErrorMsg msg)
        {
            int rows, cols;
            int i, n;
            rows = _ErrTexts.GetLength(0);
            cols = _ErrTexts.GetLength(1);
            for (i = 2; i <= rows; i++)
            {
                if (Convert.ToInt32(_ErrTexts[i,1]) == msg.errNo)
                {
                    for (n = 2; n <= cols; n++)
                    {
                        if (_ErrTexts[1, n].ToString().ToUpper() == _Language) 
                            return _ErrTexts[i, n].ToString();
                    }
                    return _ErrTexts[i, 2].ToString();
                }
            }

            return msg.formatStr;
        }
        */

        //--- TranslateFormatStr --------------------------------------------
        private static string TranslateFormatStr(RX_DigiPrint.Services.TcpIp.SErrorMsg msg)
        {
            string str = RxGlobals.Language.GetString("Error", msg.errNo.ToString());
            if (str==null) return msg.formatStr;
            else return str;
        }

        //---  property Language -----------------------
        private static string _Language;
        public static string Language
        {
            get { return _Language; }
            set { _Language = value.ToUpper(); }
        }   

        //--- Compose ---------------------------------------------
        public static string Compose(RX_DigiPrint.Services.TcpIp.SErrorMsg msg)
        {
            StringBuilder str = new StringBuilder(256);

            int f = 0;
            int a = 0;
            int n;
            char ch;
            string fstr;
            int num;

            init();
            string formatStr = TranslateFormatStr(msg);
            if (formatStr.EndsWith("\n")) formatStr = formatStr.Remove(formatStr.Length-1);
            /*
            if (msg.devType!=0)
            {
                switch (msg.devType)
                {
                    case EDeviceType.dev_gui:  str.Append      ("GUI: ");                    break;
                    case EDeviceType.dev_main: str.Append      ("main: ");                   break;
                    case EDeviceType.dev_enc:  str.Append      ("encoder: ");                break;
                    case EDeviceType.dev_clean:str.Append      ("cleaning: ");               break;
                    case EDeviceType.dev_ink:  str.AppendFormat("ink-{0}: ", msg.devNo+1);   break;
                    case EDeviceType.dev_head: str.AppendFormat("head-{0}: ", msg.devNo+1);  break;
                    case EDeviceType.dev_dry:  str.Append      ("dyrer: ");                  break;
                    case EDeviceType.dev_plc:  str.Append      ("plc: ");                    break;
                    default: break;
                }
            }
            */
            while (f<formatStr.Length)
            {
                if (formatStr[f]=='%')
                {
                    f++;
                    num = 0;
                    ch = formatStr[f++];
                    while (ch >= '0' && ch <= '9')
                    {
                        num = 10 * num + ch - '0';
                        ch = formatStr[f++];
                    }

                    switch (ch)
                    {
                    case '%':   str.Append("%");
                                break;
                    case 'c':   str.Append((char)msg.args[a++]);
                                break;

                    case 'd':
                    case 'i':   
                    case 'x':
                    case 'X':
                    case 'N':   switch (ch)
                                {
                                    case 'x': fstr = "{0:x"; break;
                                    case 'X': fstr = "{0:X"; break;
                                    case 'N': fstr = "{0:N"; break;
                                    default: fstr = "{0:d"; break;
                                }

                                if (num > 0) fstr += num.ToString();
                                fstr += "}";
                                if (msg.args[a]=='d')
                                {
                                    a++;
                                    n  = msg.args[a++];
                                    n += msg.args[a++] * 0x100;
                                    n += msg.args[a++] * 0x10000;
                                    n += msg.args[a++] * 0x1000000;
                                    str.AppendFormat(fstr, n);
                                }
                                break;

                    case 's': if (msg.args[a] == 's')
                                {
                                    a++;
                                    while (msg.args[a] != 0)
                                        str.Append((char)msg.args[a++]);
                                    a++;
                                }
                                break;
                    }
                }
                else str.Append(formatStr[f++]);
            }

            return str.ToString();
        }
    }
}
