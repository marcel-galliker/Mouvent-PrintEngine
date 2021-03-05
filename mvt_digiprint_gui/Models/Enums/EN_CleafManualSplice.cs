using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_CleafManualSplice: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_CleafManualSplice()
        {
            _List = new List<RxEnum<int>>();
            _List.Add(new RxEnum<int>( 0,  RX_DigiPrint.Resources.Language.Resources.None));
            _List.Add(new RxEnum<int>( 1,  RX_DigiPrint.Resources.Language.Resources.Sonotrode));
            _List.Add(new RxEnum<int>( 2,  RX_DigiPrint.Resources.Language.Resources.Ultrasonic));
            _List.Add(new RxEnum<int>( 3,  RX_DigiPrint.Resources.Language.Resources.CutRight));
            _List.Add(new RxEnum<int>( 4,  RX_DigiPrint.Resources.Language.Resources.CutLeft));
            _List.Add(new RxEnum<int>( 5,  RX_DigiPrint.Resources.Language.Resources.Tape));
        }

       IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
