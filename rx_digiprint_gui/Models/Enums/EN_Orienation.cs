using RX_Common;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_OrientationList: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_OrientationList()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>(0,     "0°"  ));
                _List.Add(new RxEnum<int>(90,    "90°" ));
                _List.Add(new RxEnum<int>(180,   "180°"));
                _List.Add(new RxEnum<int>(270,   "270°"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
