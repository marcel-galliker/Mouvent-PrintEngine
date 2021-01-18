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
            if (_List==null)
            {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>( 0,  "Moving"));
                _List.Add(new RxEnum<int>( 1,  "Start"));
                _List.Add(new RxEnum<int>( 2,  "End"));
                _List.Add(new RxEnum<int>( 3,  "Capping"));
                _List.Add(new RxEnum<int>( 4,  "Purge"));
                _List.Add(new RxEnum<int>( 5,  "Wipe"));
                _List.Add(new RxEnum<int>( 6,  "Manual"));
                _List.Add(new RxEnum<int>( 7,  "Stopped"));
            }
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
