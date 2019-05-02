using RX_Common;
using RX_DigiPrint.Services;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_FluidCtrlList: IEnumerable
    {
        private static List<RxEnum<EFluidCtrlMode>> _List;

        public EN_FluidCtrlList()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<EFluidCtrlMode>>();
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_undef,         "undef"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_error,         "ERROR"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_off,           "OFF"           ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_shutdown,      "Shutdown"));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_warmup,        "Warm Up"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_readyToPrint,  "Ready To Print"));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_print,         "Print"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_bleed,         "Bleed"         ));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_night,   "Flush N"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_weekend, "Flush WE"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_week,    "Flush W"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_step1,   "Flush /1"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_step2,   "Flush /2"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_step3,   "Flush /3"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_step4,   "Flush /4"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_done,    "Flush Done"    ));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_soft,    "Purge Soft"    ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge,         "Purge"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_hard,    "Purge Hard"    ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_micro,   "Purge Micro"   ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_step1,   "Purge /1"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_step2,   "Purge /2"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_step3,   "Purge /3"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_step4,   "Purge /4"      ));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe,          "Wipe"          ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wetwipe,       "Wipe Wet"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wash,          "Wipe Wash"     ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe_step1,    "Wipe /1"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe_step2,    "Wipe /2"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe_step3,    "Wipe /3"       ));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap,          "Cap"            ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap_step1,    "Cap /1"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap_step2,    "Cap /2"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap,          "Cap /3"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap_step3,    "Cap /4"         ));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_fill,          "Fill"          ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_fill_step1,    "Fill /1"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_fill_step2,    "Fill /2"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_fill_step3,    "Fill /3"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_fill_step4,    "Fill /4"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_fill_step5,    "Fill /5"       ));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_empty,         "Empty"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_empty_step1,   "Empty /1"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_empty_step2,   "Empty /2"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_empty_step3,   "Empty /3"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_empty_step4,   "Empty /4"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_empty_step5,   "Empty /5"      ));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_start,     "Calib"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_step1,     "Calib /1"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_step2,     "Calib /2"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_step3,     "Calib /3"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_step4,     "Calib /4"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_done,      "Calib Done"    ));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_test_watchdog, "Test WDG"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_test,          "Test"          ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_offset_cal,      "Cal Offset"));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_offset_cal_done, "Cal Done"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<EFluidCtrlMode>>(_List);
        }

    }
}
