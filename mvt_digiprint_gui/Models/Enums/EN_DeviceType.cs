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
            _List = new List<RxEnum<EDeviceType>>();
            _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_undef, RX_DigiPrint.Resources.Language.Resources.Undef));
            _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_gui, RX_DigiPrint.Resources.Language.Resources.Gui));
            _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_main, RX_DigiPrint.Resources.Language.Resources.Main));
            _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_plc, RX_DigiPrint.Resources.Language.Resources.Plc));
            _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_enc, RX_DigiPrint.Resources.Language.Resources.Encoder));
            _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_fluid, RX_DigiPrint.Resources.Language.Resources.Fluid));
            _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_stepper, RX_DigiPrint.Resources.Language.Resources.Stepper));
            _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_head, RX_DigiPrint.Resources.Language.Resources.Cluster));
            _List.Add(new RxEnum<EDeviceType>(EDeviceType.dev_spool, RX_DigiPrint.Resources.Language.Resources.Spooler));            
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
