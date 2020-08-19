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
        private const double TX801_MIN_HEIGHT = 1000;

        public EN_SpeedList(UInt32 maxSpeed, double imgHeight)
        {
            if (maxSpeed!=_MaxSpeed) 
            {
                int speed;
                _List.Clear();
                _MaxSpeed = (int)maxSpeed;
                CUnit unit=new CUnit("m/min");

                if (RxGlobals.PrinterProperties.Host_Name!=null && RxGlobals.PrinterProperties.Host_Name.Equals("DropWatcher")) 
                {
                    for (speed=1; speed<=_MaxSpeed; speed++)
                    {
                        _List.Add(new RxEnum<int>(speed,  string.Format("{0}", speed)));
                    }
                }
                else if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_TX801 
                    ||   RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_TX802
                    ||   RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_DP803
                    )
                {
                    if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_TX801 && imgHeight<TX801_MIN_HEIGHT) maxSpeed = 85;
                    int[] speeds = RxGlobals.InkSupply.PrintingSpeed();
                    for (int i=0; i<speeds.Length; i++)
                    {
                        if (speeds[i]!=0 && speeds[i]<=maxSpeed) _List.Add(new RxEnum<int>(speeds[i],  string.Format("{0}", Math.Round(speeds[i]*unit.Factor))));
                    }
                }
                else
                {
                    for (speed=20; ; speed+=5)
                    {
                        if (speed>=_MaxSpeed)
                        {
                            _List.Add(new RxEnum<int>(_MaxSpeed,  string.Format("{0}", Math.Round(_MaxSpeed*unit.Factor))));
                            break;
                        }
                        _List.Add(new RxEnum<int>(speed,  string.Format("{0}", Math.Round(speed*unit.Factor))));
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
