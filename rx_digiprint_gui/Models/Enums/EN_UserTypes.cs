using RX_Common;
using RX_DigiPrint.Services;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_UserTypeList: IEnumerable
    {
        private static List<RxEnum<EUserType>> _List;

        public EN_UserTypeList()
        {
           // if (_List==null)
            {
                EUserType ServiceLevel = RxGlobals.License.ServiceLevel;

                _List = new List<RxEnum<EUserType>>();
           //   _List.Add(new RxEnum<EUserType>(EUserType.usr_undef,         "undef"     ));
                _List.Add(new RxEnum<EUserType>(EUserType.usr_operator,      "operator"  ));
                if (ServiceLevel>=EUserType.usr_supervisor) _List.Add(new RxEnum<EUserType>(EUserType.usr_supervisor,    "supervisor"));
                if (ServiceLevel>=EUserType.usr_service)    _List.Add(new RxEnum<EUserType>(EUserType.usr_service,       "service"   ));
                if (ServiceLevel>=EUserType.usr_mouvent)    _List.Add(new RxEnum<EUserType>(EUserType.usr_mouvent,       "mouvent"   ));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<EUserType>>(_List);
        }

    }
}
