using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_DeviceNumbers: IEnumerable
    {
        private List<RxEnum<int>> _List = new List<RxEnum<int>>();

        public EN_DeviceNumbers(EDeviceType type, int colorCnt, int headsPerColor)
        {
            int color, cluster, cnt, no;
            int size=(int)RX_DigiPrint.Services.TcpIp.HEAD_CNT;
            switch(type)
            {
                case EDeviceType.dev_main:      break;

                case EDeviceType.dev_plc:       break;

                case EDeviceType.dev_head:      if (headsPerColor>size)
                                                {
                                                    cnt= (headsPerColor+size-1) / size;
                                                    for (color=0, no=0; color<colorCnt; color++)
                                                    {
                                                        for (cluster=0; cluster<cnt; cluster++, no++)
                                                        {
                                                            if (RxGlobals.InkSupply.List[color].InkType == null)
                                                            {
                                                                Console.WriteLine("Tried to access InkType but is was null.");
                                                                _List.Add(new RxEnum<int>(no + 1, string.Format("{0}-{1}", color + 1, cluster + 1)));
                                                            }
                                                            else 
                                                            {
                                                                try
                                                                {
                                                                    switch (RxGlobals.InkSupply.List[color].RectoVerso)
                                                                    {
                                                                        case ERectoVerso.rv_recto: _List.Add(new RxEnum<int>(no + 1, string.Format("R{0}-{1}..{2}", InkType.ColorNameShort(RxGlobals.InkSupply.List[color].InkType.ColorCode), cluster * size + 1, cluster * size + size))); break;
                                                                        case ERectoVerso.rv_verso: _List.Add(new RxEnum<int>(no + 1, string.Format("V{0}-{1}..{2}", InkType.ColorNameShort(RxGlobals.InkSupply.List[color].InkType.ColorCode), cluster * size + 1, cluster * size + size))); break;
                                                                        default: _List.Add(new RxEnum<int>(no + 1, string.Format("{0}-{1}..{2}", InkType.ColorNameShort(RxGlobals.InkSupply.List[color].InkType.ColorCode), cluster * size + 1, cluster * size + size))); break;
                                                                    }
                                                                }
                                                                catch (Exception ex)
                                                                {
                                                                    Console.WriteLine(ex.Message);
                                                                    _List.Add(new RxEnum<int>(no + 1, string.Format("{0}-{1}", color + 1, cluster + 1)));
                                                                }
                                                            }
                                                            
                                                        }
                                                    }
                                                }
                                                else if (headsPerColor==size)
                                                {
                                                    cnt= (headsPerColor+size-1) / size;
                                                    for (color=0, no=0; color<colorCnt; color++)
                                                    {
                                                        for (cluster=0; cluster<cnt; cluster++, no++)
                                                        {
                                                            try 
                                                            {
                                                                if (RxGlobals.InkSupply.List[color].InkType != null)
                                                                {
                                                                    _List.Add(new RxEnum<int>(no + 1, string.Format("{0}", InkType.ColorNameShort(RxGlobals.InkSupply.List[color].InkType.ColorCode))));
                                                                }

                                                            }
                                                            catch (Exception ex)
                                                            {
                                                                Console.WriteLine(ex.Message);
                                                                _List.Add(new RxEnum<int>(no+1, string.Format("{0}-{1}", color+1, cluster+1)));
                                                            }
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    for (color=0; color<colorCnt; color++)
                                                        _List.Add(new RxEnum<int>(color+1, string.Format("{0}", color+1)));
                                                }
                                                break;

                case EDeviceType.dev_enc:       if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_DP803)
                                                {
                                                    for (color=0; color<2; color++)
                                                        _List.Add(new RxEnum<int>(color+1, string.Format("{0}", color+1)));
                                                }
                                                break;

                case EDeviceType.dev_spool:     cnt = 8;                                
                                                for (color=1; color<cnt; color++)
                                                   _List.Add(new RxEnum<int>(color+1, string.Format("{0}", color)));
                                                break;

                case EDeviceType.dev_fluid:     cnt = (colorCnt+size-1)/size;                                
                                                for (color=0; color<cnt && color<2; color++)
                                                   _List.Add(new RxEnum<int>(color+1, string.Format("{0}", color+1)));
                                                break;

                case EDeviceType.dev_stepper:   switch (RxGlobals.PrintSystem.PrinterType)
                                                {
                                                    case EPrinterType.printer_LB701:            cnt=4; break;
                                                    case EPrinterType.printer_LB702_UV:         cnt=4; break;
                                                    case EPrinterType.printer_LB702_WB:         cnt=4; break;
                                                    case EPrinterType.printer_LH702:            cnt=4; break;
                                                    case EPrinterType.printer_DP803:            cnt=4; break;
                                                    case EPrinterType.printer_cleaf:            cnt=4; break;
                                                    case EPrinterType.printer_test_table:       cnt=1; break;
                                                    case EPrinterType.printer_TX801:            cnt=2; break;
                                                    case EPrinterType.printer_TX802:            cnt=2; break;
                                                    case EPrinterType.printer_TX404:			cnt=2; break;
                                                    case EPrinterType.printer_test_table_seon:  cnt=3; break;      
                                                    default: cnt=1; break;
                                                }
                                                for (color=0; color<cnt && color<4; color++)
                                                    _List.Add(new RxEnum<int>(color+1, string.Format("{0}", color+1)));
                                                if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_LB702_WB 
                                                || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LB702_UV)
                                                    _List.Add(new RxEnum<int>(99+1, string.Format("SA")));
                                                break;

                 case EDeviceType.dev_robot:    switch (RxGlobals.PrintSystem.PrinterType)
                                                {
                                                    case EPrinterType.printer_LB702_WB:         cnt=4; break;
                                                    default: cnt=4; break;
                                                }
                                                for (color=0; color<cnt; color++)
                                                    _List.Add(new RxEnum<int>(color+1, string.Format("{0}", color+1)));
                                                break;

           }                                                    
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

        public int Count
        {
            get { return _List.Count;}
        }

        public string GetDisplay(int idx)
        {
            return _List[idx].Display;
        }
    }
}