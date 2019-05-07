using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_CleafWinderReduction: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_CleafWinderReduction()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>( 0,  "none"));
                _List.Add(new RxEnum<int>( 1,  "Unwinder"));
                _List.Add(new RxEnum<int>( 2,  "Rewinder"));
                _List.Add(new RxEnum<int>( 3,  "ALL"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
