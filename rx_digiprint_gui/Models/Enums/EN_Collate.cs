using RX_Common;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_CollateList: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_CollateList()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>(0,    "Pages"));
                _List.Add(new RxEnum<int>(1,    "Docs" ));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
