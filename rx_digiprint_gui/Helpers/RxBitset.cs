using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Helpers
{
    public class RxBitset
    {
        //--- Set -----------------------------------------------------
        static public void Set<type>(ref UInt32 flags, type bit)
        {
            flags |= (UInt32)(0x01<<System.Convert.ToByte(bit));
        }

        //--- Clear -----------------------------------------------------
        static public void Clear<type>(ref UInt32 flags, type bit)
        {
            flags &= (UInt32)~(0x01<<System.Convert.ToByte(bit));
        }

        //--- IsSet ---------------------------------------------------
        static public Boolean IsSet<type>(UInt32 flags, type bit)
        {
            return (flags & (UInt32)(1<<System.Convert.ToByte(bit)))!=0;
        }

    }
}
