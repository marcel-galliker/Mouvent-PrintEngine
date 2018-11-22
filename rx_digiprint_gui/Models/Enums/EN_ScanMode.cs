using RX_Common;
using RX_DigiPrint.Services;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_ScanModeList: IEnumerable
    {
        private static List<RxEnum<EScanMode>> _List;

        public EN_ScanModeList()
        {
            if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_test_table)
            {
                _List = new List<RxEnum<EScanMode>>();
                _List.Add(new RxEnum<EScanMode>(EScanMode.scan_std,    "--->"));
                _List.Add(new RxEnum<EScanMode>(EScanMode.scan_rtl,    "<---"));
                _List.Add(new RxEnum<EScanMode>(EScanMode.scan_bidir,  "<-->"));
            }
            else
            {
                _List = new List<RxEnum<EScanMode>>();
                _List.Add(new RxEnum<EScanMode>(EScanMode.scan_std,    "--->"));
                _List.Add(new RxEnum<EScanMode>(EScanMode.scan_bidir,  "<-->"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<EScanMode>>(_List);
        }

    }
}
