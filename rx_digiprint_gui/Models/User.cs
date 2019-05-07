using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
    public class User :  RxBindable
    {
        public User()
        {
        }

        //--- Property UserType ---------------------------------------
        static private EUserType _UserType=EUserType.usr_operator;
        public EUserType UserType
        {
            get { return _UserType; }
            set { SetProperty(ref _UserType, value); }
        }
        
    }
}
