using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_YesNo : IEnumerable
    {
        private static List<RxEnum<bool>> _List;

        public EN_YesNo()
        {
            if (_List == null)
            {
                _List = new List<RxEnum<bool>>();
                _List.Add(new RxEnum<bool>(false, "NO"));
                _List.Add(new RxEnum<bool>(true, "YES"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<bool>>(_List);
        }

    }
}
