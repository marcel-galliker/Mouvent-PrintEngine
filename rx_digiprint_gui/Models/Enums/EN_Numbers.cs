using RX_Common;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_Numbers: IEnumerable
    {
        private List<RxEnum<int>> _List;

        public EN_Numbers(int from, int to, int step)
        {
            if (_List==null)
            {                
                _List = new List<RxEnum<int>>();
                for (int i=from; i<=to; i++)
                {
                    int n=i*step;
                    _List.Add(new RxEnum<int>(n, n.ToString()));
                }
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
