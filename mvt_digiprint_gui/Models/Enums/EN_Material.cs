using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_MaterialList: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_MaterialList()
        {
            _List = new List<RxEnum<int>>();
            _List.Add(new RxEnum<int>(0,  "ABS 1.0mm"));
            _List.Add(new RxEnum<int>(1,  "ABS 2.5mm"));
            _List.Add(new RxEnum<int>(2, RX_DigiPrint.Resources.Language.Resources.Paper));
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

        public static int DefaultValue
        {
            get { return _List[0].Value; }            
        }
    }
}
