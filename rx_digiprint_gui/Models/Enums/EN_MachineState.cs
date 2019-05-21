using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_MachineStateList: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_MachineStateList()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>( 0,  "undef"));
                _List.Add(new RxEnum<int>( 1,  "ERROR"));
                _List.Add(new RxEnum<int>( 2,  "BOOTUP"));
                _List.Add(new RxEnum<int>( 3,  "STOP"));
                _List.Add(new RxEnum<int>( 4,  "PREPARE"));
                _List.Add(new RxEnum<int>( 5,  "PAUSE"));
                _List.Add(new RxEnum<int>( 6,  "RUN"));
                _List.Add(new RxEnum<int>( 7,  "SETUP"));
                _List.Add(new RxEnum<int>( 8,  "WARMUP"));
                _List.Add(new RxEnum<int>( 9,  "WEBIN"));
                _List.Add(new RxEnum<int>(10,  "WASHING"));
                _List.Add(new RxEnum<int>(11,  "CLEANING"));
                _List.Add(new RxEnum<int>(12,  "GLUE"));
                _List.Add(new RxEnum<int>(13,  "REFERENCING"));
                _List.Add(new RxEnum<int>(14,  "SERVICE"));
                _List.Add(new RxEnum<int>(15,  "WEBOUT"));
                _List.Add(new RxEnum<int>(16,  "MAINTENANCE"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

        public string GetDisplay(int value)
        {
            foreach(RxEnum<int> item in _List)
            {
                if (item.Value==value) return item.Display;
            }
            return null;
        }
    }
}
