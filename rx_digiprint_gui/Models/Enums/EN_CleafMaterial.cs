using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_CleafMaterialList: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_CleafMaterialList()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>( 0,  "ABS 0.5 mm"));
                _List.Add(new RxEnum<int>( 1,  "ABS 1.0 mm"));
                _List.Add(new RxEnum<int>( 2,  "ABS 2.0 mm"));
                _List.Add(new RxEnum<int>( 3,  "Paper"));
            }
        }

       IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
