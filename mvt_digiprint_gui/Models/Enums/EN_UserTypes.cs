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
            _List = new List<RxEnum<EUserType>>();
            //   _List.Add(new RxEnum<EUserType>(EUserType.usr_undef,         "undef"     ));
            _List.Add(new RxEnum<EUserType>(EUserType.usr_operator, RX_DigiPrint.Resources.Language.Resources.Operator));
            _List.Add(new RxEnum<EUserType>(EUserType.usr_maintenance, RX_DigiPrint.Resources.Language.Resources.User));
            _List.Add(new RxEnum<EUserType>(EUserType.usr_service, RX_DigiPrint.Resources.Language.Resources.Service));
            _List.Add(new RxEnum<EUserType>(EUserType.usr_engineer, RX_DigiPrint.Resources.Language.Resources.Mouvent));
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<EUserType>>(_List);
        }

    }
}
