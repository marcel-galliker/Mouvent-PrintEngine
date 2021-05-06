using RX_Common;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_OffWarningPause: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_OffWarningPause()
        {
            if (_List == null)
            {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>(0, "OFF"));
                _List.Add(new RxEnum<int>(1, "WARNING"));
                _List.Add(new RxEnum<int>(2, "PAUSE"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }
    }
}
