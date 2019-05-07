using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Windows;

namespace RX_LabelComposer.Models.Enums
{
    public class BarcodeSegPerRowList: IEnumerable
    {
        private static List<RxEnum<short>> _List;
        private static BcTypeEnum _Type = 0;

        public BarcodeSegPerRowList(BcTypeEnum type)
        {
            if (type != _Type)
            {
                _Type = type;
                _List = new List<RxEnum<short>>();
                if (type == BcTypeEnum.eBC_GS1DataBarExpStacked)
                {
                    _List.Add(new RxEnum<short>(-1, "Default"));
                    for (short i = 2; i <= 22; i++)
                    {
                        _List.Add(new RxEnum<short>(i, Convert.ToString(i)));
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
