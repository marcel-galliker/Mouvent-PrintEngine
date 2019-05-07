using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_CleafManualSplice: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_CleafManualSplice()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>( 0,  "none"));
                _List.Add(new RxEnum<int>( 1,  "sonotrode"));
                _List.Add(new RxEnum<int>( 2,  "ultrasonic"));
                _List.Add(new RxEnum<int>( 3,  "cut right"));
                _List.Add(new RxEnum<int>( 4,  "cut left"));
                _List.Add(new RxEnum<int>( 5,  "tape"));
            }
        }

       IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
