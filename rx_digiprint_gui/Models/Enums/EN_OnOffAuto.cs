using RX_Common;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_DropSize: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_DropSize()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>( 1,  "S"));
                _List.Add(new RxEnum<int>( 2,  "M"));
                _List.Add(new RxEnum<int>( 3,  "L"));
            }
        }

       IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
