using System.Collections;

namespace RX_DigiPrint.Models
{
    public class DeviceNumbersEnum : IEnumerable
    {        
        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator) new NumberEnum(1,10);
        }
    }

    public class NumberEnum : IEnumerator
    {
        private int _min, _max, _current;
        public NumberEnum(int min, int max)
        {
            _min = min;
            _max = max;
            _current = min;
        }

        public object Current
        {
            get { return _current; }
        }

        public bool MoveNext()
        {
            if (_current+1<_max) 
            {
                _current++;
                return true;
            }
            return false;
        }

        public void Reset()
        {
            _current=_min;
        }
    }
}
