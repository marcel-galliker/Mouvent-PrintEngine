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
                _List.Add(new RxEnum<int>(0, "P"));
                _List.Add(new RxEnum<int>(1, "Q"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }

    public class EN_RotationList_OFF: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_RotationList_OFF()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>(0, "P"));
                _List.Add(new RxEnum<int>(1, "Q"));
                _List.Add(new RxEnum<int>(2, "_"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
