using RX_Common;
using rx_rip_gui.External;
using rx_rip_gui.Services;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_ScreeningMethod: IEnumerable
    {
        private static List<RxEnum<EScreeningMode>> _List;

        public EN_ScreeningMethod()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<EScreeningMode>>();
                _List.Add(new RxEnum<EScreeningMode>( EScreeningMode.sm_undef,  "undef"));
                _List.Add(new RxEnum<EScreeningMode>( EScreeningMode.sm_1,  "Method 1"));
                _List.Add(new RxEnum<EScreeningMode>( EScreeningMode.sm_2,  "Method 2"));
                _List.Add(new RxEnum<EScreeningMode>( EScreeningMode.sm_3,  "Method 3"));
                _List.Add(new RxEnum<EScreeningMode>( EScreeningMode.sm_4,  "Method 4"));
                /*
                int xml=RX_EmbRip.embrip_create_xml();

                StringBuilder str = new StringBuilder(32);
                _List = new List<RxEnum<EScreeningMode>>();
                for(EScreeningMode mode=EScreeningMode.sm_undef; RX_EmbRip.embrip_ScreeningModeStr(mode, str)==0; mode++)
                {
                    _List.Add(new RxEnum<EScreeningMode>( mode,  str.ToString()));
                }
                 * */

            }
        }

       IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<EScreeningMode>>(_List);
        }

    }
}
