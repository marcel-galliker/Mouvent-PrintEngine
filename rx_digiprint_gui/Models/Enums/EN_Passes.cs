using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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
                _List.Add(new RxEnum<int>(3, "3"));
                _List.Add(new RxEnum<int>(4, "4"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
