using RX_Common;
using RX_LabelComposer.Models.Enums;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Windows;

namespace RX_LabelComposer.Models.Models.Enums
{
    public class BarcodeMaskList: IEnumerable
    {
        private static List<RxEnum<short>> _List;
        private static BcTypeEnum _Type = 0;

        public BarcodeMaskList(BcTypeEnum type)
        {
            if (type != _Type)
            {
                _Type = type;
                _List = new List<RxEnum<short>>();
                if (type == BcTypeEnum.eBC_DataMatrix)
                {
                    _List.Add(new RxEnum<short>(0, "Auto"));
                    _List.Add(new RxEnum<short>(1, "Binary"));
                    _List.Add(new RxEnum<short>(2, "ASCII"));
                }
                else if (type == BcTypeEnum.eBC_QRCode)
                {
                    _List.Add(new RxEnum<short>(-1, "Default"));
                    _List.Add(new RxEnum<short>( 0, "Mask 0"));
                    _List.Add(new RxEnum<short>( 1, "Mask 1"));
                    _List.Add(new RxEnum<short>( 2, "Mask 2"));
                    _List.Add(new RxEnum<short>( 3, "Mask 3"));
                    _List.Add(new RxEnum<short>( 4, "Mask 4"));
                    _List.Add(new RxEnum<short>( 5, "Mask 5"));
                    _List.Add(new RxEnum<short>( 6, "Mask 6"));
                    _List.Add(new RxEnum<short>( 7, "Mask 7"));
                }
                else if (type == BcTypeEnum.eBC_DotCode || type == BcTypeEnum.eBC_HanXin || type == BcTypeEnum.eBC_MicroQRCode)
                {
                    _List.Add(new RxEnum<short>(-1, "Default"));
                    _List.Add(new RxEnum<short>( 0, "Mask 0"));
                    _List.Add(new RxEnum<short>( 1, "Mask 1"));
                    _List.Add(new RxEnum<short>( 2, "Mask 2"));
                    _List.Add(new RxEnum<short>( 3, "Mask 3"));
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
