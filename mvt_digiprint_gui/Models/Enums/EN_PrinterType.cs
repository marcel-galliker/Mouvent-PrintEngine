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
            _List = new List<RxEnum<EPrinterType>>();
            _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_undef,           RX_DigiPrint.Resources.Language.Resources.Undef));
            _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_test_slide,     RX_DigiPrint.Resources.Language.Resources.RadexTestSlide));
            _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_test_slide_only, RX_DigiPrint.Resources.Language.Resources.RadexTestSlideOnly));
            _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_test_table,     RX_DigiPrint.Resources.Language.Resources.Swiss4JetTestTable));
            _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_test_table_seon, RX_DigiPrint.Resources.Language.Resources.ProductionTestTable));
            _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_test_CTC,       RX_DigiPrint.Resources.Language.Resources.Test_CTC));
            _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_LB701,          RX_DigiPrint.Resources.Language.Resources.MouventLB701));
            _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_LB702_UV,       RX_DigiPrint.Resources.Language.Resources.MouventLB702UV));
            _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_LB702_WB,       RX_DigiPrint.Resources.Language.Resources.MouventLB702WB));
			_List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_LB703_UV,       RX_DigiPrint.Resources.Language.Resources.MouventLB703UV));
            _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_LH702,          RX_DigiPrint.Resources.Language.Resources.MouventLH702));

            _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_DP803,          RX_DigiPrint.Resources.Language.Resources.MouventDP803));

            _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_TX801,          RX_DigiPrint.Resources.Language.Resources.MouventTX801));
            _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_TX802,          RX_DigiPrint.Resources.Language.Resources.MouventTX802));
            _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_TX404,          RX_DigiPrint.Resources.Language.Resources.MouventTX404));

            _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_CB612,          RX_DigiPrint.Resources.Language.Resources.MouventCB612));

            _List.Add(new RxEnum<EPrinterType>(EPrinterType.printer_cleaf,          RX_DigiPrint.Resources.Language.Resources.CleafProject));
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<EPrinterType>>(_List);
        }

    }
}
