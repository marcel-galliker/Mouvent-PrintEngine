using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;

namespace RX_LabelComposer.Models.Enums
{
    public class CodePageList : IEnumerable
    {
        private static List<RxEnum<Int32>> _List;

        public CodePageList()
        {
            if (_List == null)
            {
                _List = new List<RxEnum<Int32>>();
                _List.Add(new RxEnum<Int32>(0, string.Format("{0}: {1}", 0, "Windows")));
                foreach (EncodingInfo ei in Encoding.GetEncodings())
                {
                    Encoding e = ei.GetEncoding();
                    _List.Add(new RxEnum<Int32>(e.CodePage, string.Format("{0}: {1}", e.CodePage, e.EncodingName)));
                }
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<Int32>>(_List);
        }
    }

}