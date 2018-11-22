using RX_Common;
using rx_rip_gui.Services;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_BlackPreserve: IEnumerable
    {
        private static List<RxEnum<EBlackPreserve>> _List;

        public EN_BlackPreserve()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<EBlackPreserve>>();
                _List.Add(new RxEnum<EBlackPreserve>( EBlackPreserve.bp_undef,       "undef"));
                _List.Add(new RxEnum<EBlackPreserve>( EBlackPreserve.bp_no,          "no"));
                _List.Add(new RxEnum<EBlackPreserve>( EBlackPreserve.bp_black_only,  "black only"));
                _List.Add(new RxEnum<EBlackPreserve>( EBlackPreserve.bp_black_plane, "black plane"));
            }
        }

       IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<EBlackPreserve>>(_List);
        }

    }
}
