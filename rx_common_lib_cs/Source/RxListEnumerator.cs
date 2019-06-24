using System.Collections;
using System.Collections.Generic;

namespace RX_Common
{
    public class RxListEnumerator<list_type> : IEnumerator
    {
        private int pos = -1;
        private List<list_type> _List;

        public RxListEnumerator(List<list_type> list)
        {
            _List = list;
        }

        object IEnumerator.Current
        {
            get { return _List[pos]; }
        }

        bool IEnumerator.MoveNext()
        {
            if (_List==null) return false;
            pos++;
            return (pos < _List.Count);
        }

        void IEnumerator.Reset()
        {
            pos = -1;
        }
    }

}
