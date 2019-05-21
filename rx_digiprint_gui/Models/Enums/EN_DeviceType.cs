using RX_Common;
using RX_DigiPrint.Services;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_DeviceTypeList: IEnumerable
    {
        private static List<RxEnum<EDeviceType>> _List;

        public EN_DeviceTypeList()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<EDeviceType>>();
                _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_undef,    "undef"      ));
                _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_gui,      "gui"        ));
                _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_main,     "main"       ));
                _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_plc,      "plc"        ));
                _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_enc,      "encoder"    ));
                _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_fluid,    "fluid"      ));
                _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_stepper,  "stepper"    ));
                _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_head,     "cluster"    ));
                _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_spool,    "spooler"    ));            
            }
        }

        /*
        public Visibility Visible()
        {
            return (_List.Count() > 0) ? Visibility.Visible : Visibility.Collapsed;
        }
        */

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<EDeviceType>>(_List);
        }

    }
}
