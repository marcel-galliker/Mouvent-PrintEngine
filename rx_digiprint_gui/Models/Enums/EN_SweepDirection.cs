using RX_Common;
using System.Collections;
using System.Collections.Generic;
using System.Windows.Media;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_SweepDirectionList: IEnumerable
    {
        private static List<RxEnum<SweepDirection>> _List;

        public EN_SweepDirectionList()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<SweepDirection>>();
                _List.Add(new RxEnum<SweepDirection>(SweepDirection.Counterclockwise,   "Q"  ));
                _List.Add(new RxEnum<SweepDirection>(SweepDirection.Clockwise,          "P" ));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<SweepDirection>>(_List);
        }

    }
}
