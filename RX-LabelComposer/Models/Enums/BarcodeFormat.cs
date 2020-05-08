using RX_Common;
using RX_LabelComposer.Models.Enums;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Windows;

namespace RX_LabelComposer.Models.Models.Enums
{
    public class BarcodeFormatList : IEnumerable
    {
        private static List<RxEnum<short>> _List;
        private static BcTypeEnum _Type = 0;

        public BarcodeFormatList(BcTypeEnum type)
        {
            if (type != _Type)
            {
                _Type = type;
                _List = new List<RxEnum<short>>();
                if (type == BcTypeEnum.eBC_DataMatrix)
                {
                    _List.Add(new RxEnum<short>(0, "Default"));
                    _List.Add(new RxEnum<short>(1, "UCC/EAN/GS1"));
                    _List.Add(new RxEnum<short>(2, "Industry"));
                    _List.Add(new RxEnum<short>(3, "Format 05"));
                    _List.Add(new RxEnum<short>(4, "Format 06"));
                    _List.Add(new RxEnum<short>(5, "Reader Programming"));
                    _List.Add(new RxEnum<short>(6, "DP Postmatrix"));
                }
                else if (type == BcTypeEnum.eBC_QRCode)
                {
                    _List.Add(new RxEnum<short>(0, "Default"));
                    _List.Add(new RxEnum<short>(1, "UCC/EAN/GS1"));
                    _List.Add(new RxEnum<short>(2, "Industry"));
                }
                else if (type == BcTypeEnum.eBC_Aztec)
                {
                    _List.Add(new RxEnum<short>(0, "Default"));
                    _List.Add(new RxEnum<short>(1, "UCC/EAN/GS1"));
                    _List.Add(new RxEnum<short>(2, "Industry"));
                }
                else if (type == BcTypeEnum.eBC_MicroPDF417)
                {
                    _List.Add(new RxEnum<short>(0, "Default"));
                    _List.Add(new RxEnum<short>(1, "UCC/EAN-128 Emulation"));
                    _List.Add(new RxEnum<short>(2, "Code 128 Emulation"));
                    _List.Add(new RxEnum<short>(3, "Code 128/FNC2 Emul."));
                    _List.Add(new RxEnum<short>(4, "Linked UCC/EAN-128"));
                    _List.Add(new RxEnum<short>(5, "05 Macro"));
                    _List.Add(new RxEnum<short>(6, "06 Macro"));
                    _List.Add(new RxEnum<short>(7, "CC-A Data Mode"));
                    _List.Add(new RxEnum<short>(8, "CC-B Prefix"));
                }
                else if (type == BcTypeEnum.eBC_PDF417Trunc || type==BcTypeEnum.eBC_PDF417)
                {
                    _List.Add(new RxEnum<short>(0, "Default"));
                    _List.Add(new RxEnum<short>(1, "Binary Compaction"));
                }
                else if (type == BcTypeEnum.eBC_DotCode)
                {
                    _List.Add(new RxEnum<short>(4, "Default"));
                    _List.Add(new RxEnum<short>(2, "SCM Numeric"));
                    _List.Add(new RxEnum<short>(3, "SCM Alphanumeric"));
                    _List.Add(new RxEnum<short>(4, "Alphanumeric standard ECC"));
                    _List.Add(new RxEnum<short>(5, "Alphanumeric High ECC"));
                }
                else if (type == BcTypeEnum.eBC_MAXICODE)
                {

                }
            }
        }

        public Visibility Visible()
        {
            return (_List.Count() > 0) ? Visibility.Visible : Visibility.Collapsed;
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<short>>(_List);
        }
    }
}
