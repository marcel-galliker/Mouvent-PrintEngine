using RX_Common;
using RX_DigiPrint.Services;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_RotationList: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_RotationList()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<int>>();
             //   _List.Add(new RxEnum<bool>(false, "\\..\\Resources\\Bitmaps\\Rotate_CW.ico"));
             //   _List.Add(new RxEnum<bool>(true,  "\\..\\Resources\\Bitmaps\\Rotate_CCW.ico"));
                _List.Add(new RxEnum<int>(0, "P"));
                _List.Add(new RxEnum<int>(1, "Q"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
