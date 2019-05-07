
using RX_Common;
using System.Collections;
using System.Collections.Generic;


namespace RX_LabelComposer.Models.Enums
{
    public class OrientationList : IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public OrientationList()
        {
            if (_List == null)
            {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>(0,       "0°"));
                _List.Add(new RxEnum<int>(900,     "90°"));
                _List.Add(new RxEnum<int>(1800,    "180°"));
                _List.Add(new RxEnum<int>(2700,    "270°"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }
    }
 }
