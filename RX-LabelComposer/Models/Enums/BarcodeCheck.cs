using RX_Common;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Windows;

namespace RX_LabelComposer.Models.Enums
{
    public class BarcodeCheckList : IEnumerable
    {
        private static List<RxEnum<short>> _List;
        private static BcTypeEnum _Type = 0;

        public BarcodeCheckList(BcTypeEnum type)
        {
            if (type != _Type)
            {
                _Type = type;
                _List = new List<RxEnum<short>>();
                if (type == BcTypeEnum.eBC_DataMatrix)
                {
                    _List.Add(new RxEnum<short>(0, "Default"));
                    _List.Add(new RxEnum<short>(1, "Level 1"));
                    _List.Add(new RxEnum<short>(2, "Level 2"));
                    _List.Add(new RxEnum<short>(3, "Level 3"));
                    _List.Add(new RxEnum<short>(4, "Level 4"));
                }
                else if (type == BcTypeEnum.eBC_QRCode)
                {
                    _List.Add(new RxEnum<short>(0, "Default"   ));
                    _List.Add(new RxEnum<short>(1, "(L)ow"     ));
                    _List.Add(new RxEnum<short>(2, "(M)edium"  ));
                    _List.Add(new RxEnum<short>(3, "(Q)uartil" ));
                    _List.Add(new RxEnum<short>(4, "(H)igh"    ));
                }
                else if (type == BcTypeEnum.eBC_HanXin)
                {
                    _List.Add(new RxEnum<short>(0, "Default"));
                    _List.Add(new RxEnum<short>(1, "Level 1"));
                    _List.Add(new RxEnum<short>(2, "Level 2"));
                    _List.Add(new RxEnum<short>(3, "Level 3"));
                }
                else if (type == BcTypeEnum.eBC_Aztec)
                {
                    _List.Add(new RxEnum<short>(23, "Default"));
                    for (short i = 0; i < 100; i++)
                    {
                        if (i==23)
                            _List.Add(new RxEnum<short>(i, i + "% (Default)"));
                        else
                            _List.Add(new RxEnum<short>(i, i+"%"));
                    }
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
