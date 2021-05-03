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
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_shutdown,      "Shutdown"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_shutdown_done, "SD-Done"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_error,         "ERROR"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wait,          "Wait"          ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_off,           "OFF"           ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_warmup,        "Warm Up"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_readyToPrint,  "Ready To Print"));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_print,         "Print"         ));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step0,   "Check /0"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step1,   "Check /1"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step2,   "Check /2"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step3,   "Check /3"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step4,   "Check /4"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step5,   "Check /5"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step6,   "Check /6"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step7,   "Check /7"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step8,   "Check /8"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step9,   "Check /9"      ));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_night,   "Flush N"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_weekend, "Flush WE"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_week,    "Flush W"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_step1,   "Flush /1"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_step2,   "Flush /2"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_step3,   "Flush /3"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_step4,   "Flush /4"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_done,    "Flush Done"    ));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_soft,        "Purge Soft"    ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge,             "Purge"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_hard,        "Purge Hard"    ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_hard_wipe,   "Purge+Wipe"    ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_hard_wash,   "Purge+Wash"    ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_hard_vacc,   "Purge+Vacc"    ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge4ever,        "Purge4Ever"    ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_step1,   "Purge /1"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_step2,   "Purge /2"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_step3,   "Purge /3"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_step4,   "Purge /4"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_step5,   "Purge /5"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_step6,   "Purge /6"      ));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_recovery_start, "Recovery"));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_recovery_step1, "Rec /1"));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_recovery_step2, "Rec /2"));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_recovery_step3, "Rec /3"));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_recovery_step4, "Rec /4"));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_recovery_step5, "Rec /5"));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_recovery_step6, "Rec /6"));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_recovery_step7, "Rec /7"));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_recovery_step8, "Rec /8"));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_recovery_step9, "Rec /9"));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe,          "Wipe"          ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe_step1,    "Wipe /1"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe_step2,    "Wipe /2"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe_step3,    "Wipe /3"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe_step4,    "Wipe /4"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe_step5,    "Wipe /5"       ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe_step6,    "Wipe /6"       ));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wash,          "Wash"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wash_step1,    "Wash /1"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wash_step2,    "Wash /2"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wash_step3,    "Wash /3"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wash_step4,    "Wash /4"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wash_step5,    "Wash /5"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wash_step6,    "Wash /6"      ));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap,          "Cap"            ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap_step1,    "Cap /1"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap_step2,    "Cap /2"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap_step3,    "Cap /3"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap_step4,    "Cap /4"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap_step5,    "Cap /5"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap_step6,    "Cap /6"         ));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum,       "Vac"            ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step1, "Vac /1"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step2, "Vac /2"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step3, "Vac /3"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step4, "Vac /4"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step5, "Vac /5"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step6, "Vac /6"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step7, "Vac /7"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step8, "Vac /8"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step9, "Vac /9"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step10,"Vac /10"        ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step11,"Vac /11"        ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step12,"Vac /12"        ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step13,"Vac /13"        ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step14,"Vac /14"        ));

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
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_empty_step6,   "Empty /6"      ));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_start,     "Calib"         ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_step1,     "Calib /1"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_step2,     "Calib /2"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_step3,     "Calib /3"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_step4,     "Calib /4"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_done,      "Calib Done"    ));

                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_test_watchdog, "Test WDG"      ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_test,          "Test"          ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_offset_cal,      "Cal Offset"  ));
                _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_offset_cal_done, "Cal Done"    ));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<EFluidCtrlMode>>(_List);
        }

    }
}
