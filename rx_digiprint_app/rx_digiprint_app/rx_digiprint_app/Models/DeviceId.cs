using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DigiPrint.Models
{
    public class DeviceId
    {
        public string Manufacturer;
        public string Model;
        public string Serial;
        public string UserName;

        public override string ToString()
        {
            return  "Phone: " +AppGlobals.DeviceId.Manufacturer   +"\n"+
                    "Model: " +AppGlobals.DeviceId.Model          +"\n"+
                    "Serial: " +AppGlobals.DeviceId.Serial         +"\n"+
                    "UserName: " +AppGlobals.DeviceId.UserName;
        }

        public string Info()
        {
            return  AppGlobals.DeviceId.Manufacturer   +"\n"+
                    AppGlobals.DeviceId.Model          +"\n"+
                    AppGlobals.DeviceId.Serial         +"\n"+
                    AppGlobals.DeviceId.UserName;
        }

    }
}
