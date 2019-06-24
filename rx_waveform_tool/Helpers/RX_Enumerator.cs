using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_GUI.Helpers
{
    public class RX_Enumerator<list_type> : IEnumerator
    {
        private int pos = -1;
        private List<list_type> _List;

        public RX_Enumerator(List<list_type> list)
        {
            _List = list;
        }

        object IEnumerator.Current
        {
            get { return _List[pos]; }
        }

        bool IEnumerator.MoveNext()
        {
            pos++;
            return (pos < _List.Count);
        }

        void IEnumerator.Reset()
        {
            pos = -1;
        }
    }
}
