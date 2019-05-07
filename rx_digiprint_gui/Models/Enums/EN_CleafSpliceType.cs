using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_CleafSpliceType: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_CleafSpliceType()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>( 0,  "Knife small / Tape"));
                _List.Add(new RxEnum<int>( 1,  "Knife small / Ultrasonic"));
                _List.Add(new RxEnum<int>( 2,  "Knife large / Tape"));
                _List.Add(new RxEnum<int>( 3,  "Knife large / Ultrasonic"));
            }
        }

       IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
