using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Windows;

namespace RX_LabelComposer.Models.Enums
{
    public class BarcodeRowsList: IEnumerable
    {
        private static List<RxEnum<short>> _List;
        private static BcTypeEnum _Type = 0;

        public BarcodeRowsList(BcTypeEnum type)
        {
            if (type != _Type)
            {
                _Type = type;
                _List = new List<RxEnum<short>>();
                if (type == BcTypeEnum.eBC_PDF417 || type == BcTypeEnum.eBC_PDF417Trunc)
                {
                    _List.Add(new RxEnum<short>(-1, "Default"));
                    for (short i = 3; i <= 90; i++)
                    {
                        _List.Add(new RxEnum<short>(i, Convert.ToString(i)));
                    }
                }
            }
        }
       
        public Visibility Visible()
        {
            return (_List.Count() > 0)?  Visibility.Visible: Visibility.Collapsed;
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<short>>(_List);
        }
    }
}
