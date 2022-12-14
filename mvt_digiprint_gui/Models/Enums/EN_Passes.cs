using RX_Common;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{

	public class EN_PassesList : IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_PassesList()
        {
            if (_List == null)
            {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>(1, "1"));
                _List.Add(new RxEnum<int>(2, "2"));
                _List.Add(new RxEnum<int>(4, "4"));
                _List.Add(new RxEnum<int>(0x12, "V 2"));
                _List.Add(new RxEnum<int>(0x14, "V 4"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
