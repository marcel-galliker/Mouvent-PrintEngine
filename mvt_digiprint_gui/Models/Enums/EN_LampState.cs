using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_LampStateList: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_LampStateList()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>( 0,  "OFF"));
                _List.Add(new RxEnum<int>( 1,  "ON"));
                _List.Add(new RxEnum<int>( 2,  "FLASH"));
            }
        }

       IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
