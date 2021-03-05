using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_MachineStateList: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_MachineStateList()
        {
            _List = new List<RxEnum<int>>();
            _List.Add(new RxEnum<int>( 0, RX_DigiPrint.Resources.Language.Resources.Undef));
            _List.Add(new RxEnum<int>( 1, RX_DigiPrint.Resources.Language.Resources.Error));
            _List.Add(new RxEnum<int>( 2, RX_DigiPrint.Resources.Language.Resources.BootUp));
            _List.Add(new RxEnum<int>( 3, RX_DigiPrint.Resources.Language.Resources.Stop));
            _List.Add(new RxEnum<int>( 4, RX_DigiPrint.Resources.Language.Resources.Prepare));
            _List.Add(new RxEnum<int>( 5, RX_DigiPrint.Resources.Language.Resources.Pause));
            _List.Add(new RxEnum<int>( 6, RX_DigiPrint.Resources.Language.Resources.Run));
            _List.Add(new RxEnum<int>( 7, RX_DigiPrint.Resources.Language.Resources.Setup));
            _List.Add(new RxEnum<int>( 8, RX_DigiPrint.Resources.Language.Resources.WarmUp));
            _List.Add(new RxEnum<int>( 9, RX_DigiPrint.Resources.Language.Resources.WebIn));
            _List.Add(new RxEnum<int>(10, RX_DigiPrint.Resources.Language.Resources.Washing));
            _List.Add(new RxEnum<int>(11, RX_DigiPrint.Resources.Language.Resources.Cleaning));
            _List.Add(new RxEnum<int>(12, RX_DigiPrint.Resources.Language.Resources.Glue));
            _List.Add(new RxEnum<int>(13, RX_DigiPrint.Resources.Language.Resources.Referencing));
            _List.Add(new RxEnum<int>(14, RX_DigiPrint.Resources.Language.Resources.Service));
            _List.Add(new RxEnum<int>(15, RX_DigiPrint.Resources.Language.Resources.WebOut));
            _List.Add(new RxEnum<int>(16, RX_DigiPrint.Resources.Language.Resources.Maintenance));
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

        public string GetDisplay(int value)
        {
            foreach(RxEnum<int> item in _List)
            {
                if (item.Value==value) return item.Display;
            }
            return null;
        }
    }
}
