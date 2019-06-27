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
            if (RxGlobals.PrinterProperties.Host_Name.Equals("LB701-0001"))
            {
                int speed;
                for (speed=20; speed<110; speed++)
                {
                    _List.Add(new RxEnum<int>(speed,  string.Format("{0}", speed)));
                }
                return;
            }

            if (maxSpeed!=_MaxSpeed) 
            {
                int speed;
                int step=5;
                while(_List.Count()>0) _List.RemoveAt(0);
                _MaxSpeed = (int)maxSpeed;

                if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_TX801 || RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_TX802)
                {
                    int[] speeds={30,60,85,100};
                    int[] speeds1 = RxGlobals.InkSupply.PrintingSpeed();
                    for (int i=0; i<speeds.Length; i++)
                    {
                        if (speeds[i]!=0 && speeds[i]<=maxSpeed) _List.Add(new RxEnum<int>(speeds[i],  string.Format("{0}", speeds[i])));
                    }
                }
                else
                {
                    if (RxGlobals.PrinterProperties.Host_Name.Equals("DropWatcher")) step=1;
                    for (speed=20; ; speed+=step)
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
