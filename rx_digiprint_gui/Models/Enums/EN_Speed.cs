using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_SpeedList: IEnumerable
    {
        private List<RxEnum<int>> _List=new List<RxEnum<int>>();

        private int _MaxSpeed = 0;

        public EN_SpeedList(UInt32 maxSpeed)
        {
            if (maxSpeed!=_MaxSpeed) 
            {
                int speed;
                int step=5;
                while(_List.Count()>0) _List.RemoveAt(0);
                _MaxSpeed = (int)maxSpeed;
                /*
                _List.Add(new RxEnum<int>(1,  string.Format("{0}", 1)));
                _List.Add(new RxEnum<int>(2,  string.Format("{0}", 2)));
                _List.Add(new RxEnum<int>(5,  string.Format("{0}", 5)));
                */
//                for (speed=10; ; speed+=10)

                if (RxGlobals.PrinterProperties.Host_Name.Equals("DropWatcher")) step=1;

                for (speed=10; ; speed+=step)
                {
                    if (speed>=_MaxSpeed)
                    {
                        _List.Add(new RxEnum<int>(_MaxSpeed,  string.Format("{0}", _MaxSpeed)));
                        break;
                    }
                    _List.Add(new RxEnum<int>(speed,  string.Format("{0}", speed)));
                }
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

        public static int DefaultValue
        {
            get 
            {
                if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_test_table) return EN_SpeedTTList.DefaultValue;
                return 30;
            }            
        }

    }
}
