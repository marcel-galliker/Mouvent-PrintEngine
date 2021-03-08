using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_MachineScannerPosList: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_MachineScannerPosList()
        {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>( 0,  RX_DigiPrint.Resources.Language.Resources.Moving));
                _List.Add(new RxEnum<int>( 1,  RX_DigiPrint.Resources.Language.Resources.Start));
                _List.Add(new RxEnum<int>( 2,  RX_DigiPrint.Resources.Language.Resources.End));
                _List.Add(new RxEnum<int>( 3,  RX_DigiPrint.Resources.Language.Resources.Capping));
                _List.Add(new RxEnum<int>( 4,  RX_DigiPrint.Resources.Language.Resources.Purge));
                _List.Add(new RxEnum<int>( 5,  RX_DigiPrint.Resources.Language.Resources.Wipe));
                _List.Add(new RxEnum<int>( 6,  RX_DigiPrint.Resources.Language.Resources.Manual));
                _List.Add(new RxEnum<int>( 7,  RX_DigiPrint.Resources.Language.Resources.Standstill));
                _List.Add(new RxEnum<int>( 8,  RX_DigiPrint.Resources.Language.Resources.FillCap));
                _List.Add(new RxEnum<int>( 9,  RX_DigiPrint.Resources.Language.Resources.Stop));
        }

       IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

        public string GetDisplay(int value)
        {
            foreach (RxEnum<int> item in _List)
            {
                if (item.Value == value) return item.Display;
            }
            return null;
        }

    }
}
