using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_RectoVerso : IEnumerable
    {
        private static List<RxEnum<ERectoVerso>> _List;

        public EN_RectoVerso()
        {
            if (_List == null)
            {
                _List = new List<RxEnum<ERectoVerso>>();
                _List.Add(new RxEnum<ERectoVerso>(ERectoVerso.rv_recto, "Recto"));
                _List.Add(new RxEnum<ERectoVerso>(ERectoVerso.rv_verso, "Verso"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<ERectoVerso>>(_List);
        }

    }
}
