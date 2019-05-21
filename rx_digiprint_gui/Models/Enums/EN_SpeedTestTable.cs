using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_SpeedTTList: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_SpeedTTList()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>(0,  "9"));
                _List.Add(new RxEnum<int>(1,  "15"));
                _List.Add(new RxEnum<int>(2,  "22.5"));
                _List.Add(new RxEnum<int>(3,  "30"));
                _List.Add(new RxEnum<int>(4,  "37.5"));
                _List.Add(new RxEnum<int>(5,  "45"));
                _List.Add(new RxEnum<int>(6,  "52.5"));
                _List.Add(new RxEnum<int>(7,  "60"));
                /*
                _List.Add(new RxEnum<int>(8,  "8"));
                _List.Add(new RxEnum<int>(9,  "9"));
                _List.Add(new RxEnum<int>(10, "10"));
                _List.Add(new RxEnum<int>(11, "11"));
                _List.Add(new RxEnum<int>(12, "12"));
                _List.Add(new RxEnum<int>(13, "13"));
                _List.Add(new RxEnum<int>(14, "14"));
                _List.Add(new RxEnum<int>(15, "15"));
                */
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

        public static int DefaultValue
        {
            get {return 5;}            
        }

    }

}
