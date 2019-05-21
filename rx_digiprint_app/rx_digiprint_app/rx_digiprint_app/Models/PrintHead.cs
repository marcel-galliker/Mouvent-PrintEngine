using DigiPrint.Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xamarin.Forms;
using static RX_DigiPrint.Services.RxBtDef;

namespace DigiPrint.Models
{
    public class PrintHead : RxBindable
    {                
        //--- PrintHead ----------------------
        public PrintHead(int no)
        {
            _no=no;
        }

        //--- Property no ---------------------------------------
        private int _no;
        public int no
        {
            get { return _no; }
        }

        //--- _intToStr ------------------
        private string _intToStr(Int32 value)
        {
            if (value==RxBtDef.INVALID_VALUE) return "-----";
            return string.Format("{0}", (value/1000).ToString());
        }

        //--- Update --------------------------------------------
        public void Update(RxBtDef.SBtHeadStat msg)
        {
            Name     = msg.name;
            Color    = Color.FromUint(msg.color);
            ColorFG  = Color.FromUint(msg.colorFG);
            Meniscus = _intToStr(msg.meniscus);
            Temp     = _intToStr(msg.temp);
            CtrlMode = msg.ctrlMode;
        }

        //--- Property Name ---------------------------------------
        private string _Name;
        public string Name
        {
            get { return _Name; }
            set { SetProperty(ref _Name, value); }
        }

        //--- Property Color ---------------------------------------
        private Color _Color;
        public Color Color
        {
            get { return _Color; }
            set { SetProperty(ref _Color, value); }
        }
        //--- Property Color ---------------------------------------
        private Color _ColorFG;
        public Color ColorFG
        {
            get { return _ColorFG; }
            set { SetProperty(ref _ColorFG, value); }
        }

        //--- Property Temp ---------------------------------------
        private string _Temp;
        public string Temp
        {
            get { return _Temp; }
            set { SetProperty(ref _Temp, value); }
        }

        //--- Property Meniscus ---------------------------------------
        private string _Meniscus;
        public string Meniscus
        {
            get { return _Meniscus; }
            set { SetProperty(ref _Meniscus, value); }
        }

        //--- Property CtrlMode ---------------------------------------
        private EFluidCtrlMode _CtrlMode;
        public EFluidCtrlMode CtrlMode
        {
            get { return _CtrlMode; }
            set 
            { 
                if (SetProperty(ref _CtrlMode, value))
                {
                    switch(_CtrlMode)
                    {
                    case EFluidCtrlMode.ctrl_undef:         CtrlModeStr = "???";      break;
                    case EFluidCtrlMode.ctrl_shutdown:      CtrlModeStr = "SD";       break;
                    case EFluidCtrlMode.ctrl_off:           CtrlModeStr = "OFF";      break;
                    case EFluidCtrlMode.ctrl_print:         CtrlModeStr = "Print";    break;
 //                   case EFluidCtrlMode.ctrl_flush:         CtrlModeStr = "Flush";    break;
                                                            
                    case EFluidCtrlMode.ctrl_purge_soft:    CtrlModeStr = "Prg S";   break;
                    case EFluidCtrlMode.ctrl_purge:         CtrlModeStr = "Purge";    break;
                    case EFluidCtrlMode.ctrl_purge_hard:    CtrlModeStr = "Prg H";   break;
                    case EFluidCtrlMode.ctrl_purge_micro:   CtrlModeStr = "Prg M";   break;
                    case EFluidCtrlMode.ctrl_purge_step1:   CtrlModeStr = "Prg-1";  break;
                    case EFluidCtrlMode.ctrl_purge_step2:   CtrlModeStr = "Prg-2";  break;
                    case EFluidCtrlMode.ctrl_purge_step3:   CtrlModeStr = "Prg-3";  break;
                    case EFluidCtrlMode.ctrl_wipe:          CtrlModeStr = "Wipe";     break;
                                                            
                    case EFluidCtrlMode.ctrl_fill:          CtrlModeStr = "Fill";     break;
                    case EFluidCtrlMode.ctrl_fill_step1:    CtrlModeStr = "Fill-1";   break;
                    case EFluidCtrlMode.ctrl_fill_step2:    CtrlModeStr = "Fill-2";   break;
                    case EFluidCtrlMode.ctrl_fill_step3:    CtrlModeStr = "Fill-3";   break;
                    case EFluidCtrlMode.ctrl_fill_step4:    CtrlModeStr = "Fill-4";   break;
                    case EFluidCtrlMode.ctrl_fill_step5:    CtrlModeStr = "Fill-5";   break;

                    case EFluidCtrlMode.ctrl_empty:         CtrlModeStr = "Empty";    break;
                    case EFluidCtrlMode.ctrl_empty_step1:   CtrlModeStr = "Empty/1";  break;
                    case EFluidCtrlMode.ctrl_empty_step2:   CtrlModeStr = "Empty/2";  break;
                    case EFluidCtrlMode.ctrl_empty_step3:   CtrlModeStr = "Empty/3";  break;
                    case EFluidCtrlMode.ctrl_empty_step4:   CtrlModeStr = "Empty/4";  break;
                    case EFluidCtrlMode.ctrl_empty_step5:   CtrlModeStr = "Empty/5";  break;

                    default:                                CtrlModeStr = "???"; break;
                    }
                }
            }
        }

        //--- Property ControlModeStr ---------------------------------------
        private string _CtrlModeStr="???";
        public string CtrlModeStr
        {
            get { return _CtrlModeStr; }
            set { SetProperty(ref _CtrlModeStr, value); }
        }

        //--- Property IsSelected ---------------------------------------
        private bool _IsSelected=false;
        public bool IsSelected
        {
            get { return _IsSelected; }
            set { SetProperty(ref _IsSelected, value); }
        }

    }
}
