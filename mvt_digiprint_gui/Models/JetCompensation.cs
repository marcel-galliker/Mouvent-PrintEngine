using RX_Common;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
    public class JetCompensation : RxBindable
    {
        public int JetNumber { get; set; }

        // --- for xam grid only: -------------------- //
        public int Index { get; set; }

        public bool Remove
        {
            get { return true; }
        }
    }
}
