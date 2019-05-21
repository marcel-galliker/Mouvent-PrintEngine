using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_MachineWarnList: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_MachineWarnList()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>( 0,  "Main: Warning"));
                _List.Add(new RxEnum<int>( 1,  "Unwinder: Web broken"));
                _List.Add(new RxEnum<int>( 3,  "Unwinder: Overtension on Web"));
                _List.Add(new RxEnum<int>( 2,  "Buffer:   W2041"));
                _List.Add(new RxEnum<int>( 4,  "Buffer:   W2042"));
                _List.Add(new RxEnum<int>( 5,  "Pressure Roller: up/down Problem"));
                _List.Add(new RxEnum<int>( 6,  "Pressure Roller: forw/backw Problem"));
                _List.Add(new RxEnum<int>( 7,  "Washingunit: Up/Down movement Problem"));
                _List.Add(new RxEnum<int>( 8,  "Washingunit: Brush movement Problem"));
                _List.Add(new RxEnum<int>( 9,  "AirKnife: Drive of Blower displays Error"));
                _List.Add(new RxEnum<int>(10,  "Dryer: CAUTION Dryer overheated"));
                _List.Add(new RxEnum<int>(11,  "Dryer: Web Temperature over Limit"));
                _List.Add(new RxEnum<int>(12,  "Dryer: Drive of Blower displays Error"));
                _List.Add(new RxEnum<int>(13,  "Dryer: Dryer Hood is not closed"));
                _List.Add(new RxEnum<int>(14,  "Rocker: Mean Value Max achieved"));
                _List.Add(new RxEnum<int>(15,  "Rocker: Mean Value Min achieved"));
                _List.Add(new RxEnum<int>(16,  "Rocker: Sensor 1 Max achieved"));
                _List.Add(new RxEnum<int>(17,  "Rocker: Sensor 1 Min achieved"));
                _List.Add(new RxEnum<int>(18,  "Rocker: Sensor 2 Max achieved"));
                _List.Add(new RxEnum<int>(19,  "Rocker: Sensor 2 Min achieved"));
                _List.Add(new RxEnum<int>(20,  "Rocker: Sensor 3 Max achieved"));
                _List.Add(new RxEnum<int>(21,  "Rocker: Sensor 3 Min achieved"));
                _List.Add(new RxEnum<int>(22,  "Rewinder: W5021"));
                _List.Add(new RxEnum<int>(23,  "Rewinder: W5022"));
                _List.Add(new RxEnum<int>(24,  ""));
                _List.Add(new RxEnum<int>(25,  ""));
                _List.Add(new RxEnum<int>(26,  ""));
                _List.Add(new RxEnum<int>(27,  ""));
                _List.Add(new RxEnum<int>(28,  ""));
                _List.Add(new RxEnum<int>(29,  ""));
                _List.Add(new RxEnum<int>(30,  ""));
                _List.Add(new RxEnum<int>(31,  ""));
            }
        }

       IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
