using RX_Common;
using RX_DigiPrint.Services;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_EncoderTypeList : IEnumerable
    {
        private static List<RxEnum<EEncoderType>> _List;

        public EN_EncoderTypeList()
        {
            if (_List == null)
            {
                _List = new List<RxEnum<EEncoderType>>();
                _List.Add(new RxEnum<EEncoderType>(EEncoderType.enc_Balluff, "Balluff"));
                _List.Add(new RxEnum<EEncoderType>(EEncoderType.enc_Renishaw, "Renishaw"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<EEncoderType>>(_List);
        }

    }
}
