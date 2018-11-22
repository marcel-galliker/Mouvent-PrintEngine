
using RX_Common;
using RX_LabelComposer.External;
using System.Collections;
using System.Collections.Generic;


namespace RX_LabelComposer.Models.Enums
{
    public class IncrementList : IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public IncrementList()
        {
            if (_List == null)
            {
                _List = new List<RxEnum<int>>();
                _List.Add(new RxEnum<int>((int)IncrementEnum.INC_Undef,      "not defined"));
                _List.Add(new RxEnum<int>((int)IncrementEnum.INC_perLabel,   "Increment per Label"));
                _List.Add(new RxEnum<int>((int)IncrementEnum.INC_perRow,     "Increment per Row"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }
    }
 }
