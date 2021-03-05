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
            _List = new List<RxEnum<int>>();
            _List.Add(new RxEnum<int>( 0, RX_DigiPrint.Resources.Language.Resources.KnifeSmallTape));
            _List.Add(new RxEnum<int>( 1, RX_DigiPrint.Resources.Language.Resources.KnifeSmallUltrasonic));
            _List.Add(new RxEnum<int>( 2, RX_DigiPrint.Resources.Language.Resources.KnifeLargeTape));
            _List.Add(new RxEnum<int>( 3, RX_DigiPrint.Resources.Language.Resources.KnifeLargeUltrasonic));
        }

       IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
