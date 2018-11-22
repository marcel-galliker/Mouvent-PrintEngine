using RX_Common;
using RX_DigiPrint.Services;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_RotationList: IEnumerable
    {
        private static List<RxEnum<bool>> _List;

        public EN_RotationList()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<bool>>();
             //   _List.Add(new RxEnum<bool>(false, "\\..\\Resources\\Bitmaps\\Rotate_CW.ico"));
             //   _List.Add(new RxEnum<bool>(true,  "\\..\\Resources\\Bitmaps\\Rotate_CCW.ico"));
                _List.Add(new RxEnum<bool>(false, "P"));
                _List.Add(new RxEnum<bool>(true,  "Q"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<bool>>(_List);
        }

    }
}
