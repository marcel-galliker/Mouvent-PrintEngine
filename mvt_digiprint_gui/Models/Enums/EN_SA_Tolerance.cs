using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class SA_Tolerance_List : IEnumerable
    {
        private static List<RxEnum<double>> _List;

        public SA_Tolerance_List()
        {
            if (_List == null)
            {
                _List = new List<RxEnum<double>>();
                _List.Add(new RxEnum<double>(0.50,  "0.50"));
                _List.Add(new RxEnum<double>(0.75,  "0.75"));
                _List.Add(new RxEnum<double>(1.00,  "1.00"));
                _List.Add(new RxEnum<double>(1.25,  "1.25"));
                _List.Add(new RxEnum<double>(1.50,  "1.50"));
                _List.Add(new RxEnum<double>(2.00,  "2.00"));
                _List.Add(new RxEnum<double>(100.00, "OFF"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<double>>(_List);
        }

    }
}
