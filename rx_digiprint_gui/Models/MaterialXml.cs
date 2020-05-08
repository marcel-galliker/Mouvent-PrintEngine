using RX_Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
    public class MaterialXml : RxBindable
    {
        public MaterialXml()
        {
        }

        //--- Property PrintHeight ---------------------------------------
        private double _PrintHeight;
        public double PrintHeight
        {
            get { return _PrintHeight; }
            set { SetProperty(ref _PrintHeight, value); }
        }
        
    }
}
