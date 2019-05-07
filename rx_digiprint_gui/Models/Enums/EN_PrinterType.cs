using RX_Common;
using RX_DigiPrint.Services;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_PrinterTypeList: IEnumerable
    {
        private static List<RxEnum<EPrinterType>> _List;

        public EN_PrinterTypeList()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<EPrinterType>>();
                _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_undef,              "undef"             ));
                _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_test_slide,         "radex Test Slide"));
                _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_test_slide_only,    "radex Test Slide ONLY"));
                _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_test_table,         "Swiss4Jet Test Table"));
                _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_LB701,              "Mouvent LB701"   ));
                _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_LB702_UV,           "Mouvent LB702-UV"   ));
                _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_LB702_WB,           "Mouvent LB702-WB"   ));

                _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_DP803,              "Mouvent DP803"   ));

                _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_TX801,              "Mouvent TX801" ));
                _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_TX802,              "Mouvent TX802" ));

                _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_cleaf,              "Cleaf Project" ));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<EPrinterType>>(_List);
        }

    }
}
