using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_SpreadRollerList : IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_SpreadRollerList()
        {
            _List = new List<RxEnum<int>>();
            _List.Add(new RxEnum<int>(0, RX_DigiPrint.Resources.Language.Resources.No));
            _List.Add(new RxEnum<int>(1, RX_DigiPrint.Resources.Language.Resources.Mounted));
            _List.Add(new RxEnum<int>(2, RX_DigiPrint.Resources.Language.Resources.Running));
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
