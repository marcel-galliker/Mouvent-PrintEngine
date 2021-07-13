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
            _List = new List<RxEnum<EFluidCtrlMode>>();
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_undef, RX_DigiPrint.Resources.Language.Resources.Undef));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_shutdown, RX_DigiPrint.Resources.Language.Resources.ShutDown));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_shutdown_done, RX_DigiPrint.Resources.Language.Resources.SDDone));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_error, RX_DigiPrint.Resources.Language.Resources.Error));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wait, RX_DigiPrint.Resources.Language.Resources.Wait));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_off, RX_DigiPrint.Resources.Language.Resources.Off));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_warmup, RX_DigiPrint.Resources.Language.Resources.WarmUp));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_readyToPrint, RX_DigiPrint.Resources.Language.Resources.ReadyToPrint));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_prepareToPrint, RX_DigiPrint.Resources.Language.Resources.PrepareToPrint));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_print, RX_DigiPrint.Resources.Language.Resources.Print));

            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step0, RX_DigiPrint.Resources.Language.Resources.Check0));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step1, RX_DigiPrint.Resources.Language.Resources.Check1));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step2, RX_DigiPrint.Resources.Language.Resources.Check2));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step3, RX_DigiPrint.Resources.Language.Resources.Check3));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step4, RX_DigiPrint.Resources.Language.Resources.Check4));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step5, RX_DigiPrint.Resources.Language.Resources.Check5));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step6, RX_DigiPrint.Resources.Language.Resources.Check6));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step7, RX_DigiPrint.Resources.Language.Resources.Check7));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step8, RX_DigiPrint.Resources.Language.Resources.Check8));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_check_step9, RX_DigiPrint.Resources.Language.Resources.Check9));

            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_night, RX_DigiPrint.Resources.Language.Resources.FlushN));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_weekend, RX_DigiPrint.Resources.Language.Resources.FlushWE));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_week, RX_DigiPrint.Resources.Language.Resources.FlushW));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_step1, RX_DigiPrint.Resources.Language.Resources.Flush1));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_step2, RX_DigiPrint.Resources.Language.Resources.Flush2));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_step3, RX_DigiPrint.Resources.Language.Resources.Flush3));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_step4, RX_DigiPrint.Resources.Language.Resources.Flush4));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_flush_done, RX_DigiPrint.Resources.Language.Resources.FlushDone));

            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_soft, RX_DigiPrint.Resources.Language.Resources.PurgeSoft));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge, RX_DigiPrint.Resources.Language.Resources.Purge));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_hard, RX_DigiPrint.Resources.Language.Resources.PurgeHard));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_hard_wipe, RX_DigiPrint.Resources.Language.Resources.PurgeWipe));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_hard_vacc, RX_DigiPrint.Resources.Language.Resources.PurgeVacc));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_step1, RX_DigiPrint.Resources.Language.Resources.Purge1));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_step2, RX_DigiPrint.Resources.Language.Resources.Purge2));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_step3, RX_DigiPrint.Resources.Language.Resources.Purge3));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_step4, RX_DigiPrint.Resources.Language.Resources.Purge4));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_step5, RX_DigiPrint.Resources.Language.Resources.Purge5));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_purge_step6, RX_DigiPrint.Resources.Language.Resources.Purge6));

            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe, RX_DigiPrint.Resources.Language.Resources.Wipe));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe_step1, RX_DigiPrint.Resources.Language.Resources.Wipe1));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe_step2, RX_DigiPrint.Resources.Language.Resources.Wipe2));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe_step3, RX_DigiPrint.Resources.Language.Resources.Wipe3));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe_step4, RX_DigiPrint.Resources.Language.Resources.Wipe4));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe_step5, RX_DigiPrint.Resources.Language.Resources.Wipe5));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wipe_step6, RX_DigiPrint.Resources.Language.Resources.Wipe6));

            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wash, RX_DigiPrint.Resources.Language.Resources.Wash));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wash_step1, RX_DigiPrint.Resources.Language.Resources.Wash1));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wash_step2, RX_DigiPrint.Resources.Language.Resources.Wash2));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wash_step3, RX_DigiPrint.Resources.Language.Resources.Wash3));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wash_step4, RX_DigiPrint.Resources.Language.Resources.Wash4));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wash_step5, RX_DigiPrint.Resources.Language.Resources.Wash5));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_wash_step6, RX_DigiPrint.Resources.Language.Resources.Wash6));

            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap, RX_DigiPrint.Resources.Language.Resources.Cap));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap_step1, RX_DigiPrint.Resources.Language.Resources.Cap1));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap_step2, RX_DigiPrint.Resources.Language.Resources.Cap2));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap_step3, RX_DigiPrint.Resources.Language.Resources.Cap3));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap_step4, RX_DigiPrint.Resources.Language.Resources.Cap4));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap_step5, RX_DigiPrint.Resources.Language.Resources.Cap5));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cap_step6, RX_DigiPrint.Resources.Language.Resources.Cap6));

            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum, RX_DigiPrint.Resources.Language.Resources.Vac));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step1, RX_DigiPrint.Resources.Language.Resources.Vac1));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step2, RX_DigiPrint.Resources.Language.Resources.Vac2));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step3, RX_DigiPrint.Resources.Language.Resources.Vac3));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step4, RX_DigiPrint.Resources.Language.Resources.Vac4));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step5, RX_DigiPrint.Resources.Language.Resources.Vac5));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step6, RX_DigiPrint.Resources.Language.Resources.Vac6));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step7, RX_DigiPrint.Resources.Language.Resources.Vac7));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step8, RX_DigiPrint.Resources.Language.Resources.Vac8));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step9, RX_DigiPrint.Resources.Language.Resources.Vac9));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step10, RX_DigiPrint.Resources.Language.Resources.Vac10));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step11, RX_DigiPrint.Resources.Language.Resources.Vac11));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step12, RX_DigiPrint.Resources.Language.Resources.Vac12));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step13, RX_DigiPrint.Resources.Language.Resources.Vac13));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_vacuum_step14, RX_DigiPrint.Resources.Language.Resources.Vac14));

            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_fill, RX_DigiPrint.Resources.Language.Resources.Fill));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_fill_step1, RX_DigiPrint.Resources.Language.Resources.Fill1));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_fill_step2, RX_DigiPrint.Resources.Language.Resources.Fill2));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_fill_step3, RX_DigiPrint.Resources.Language.Resources.Fill3));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_fill_step4, RX_DigiPrint.Resources.Language.Resources.Fill4));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_fill_step5, RX_DigiPrint.Resources.Language.Resources.Fill5));

            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_empty, RX_DigiPrint.Resources.Language.Resources.Empty));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_empty_step1, RX_DigiPrint.Resources.Language.Resources.Empty1));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_empty_step2, RX_DigiPrint.Resources.Language.Resources.Empty2));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_empty_step3, RX_DigiPrint.Resources.Language.Resources.Empty3));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_empty_step4, RX_DigiPrint.Resources.Language.Resources.Empty4));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_empty_step5, RX_DigiPrint.Resources.Language.Resources.Empty5));

            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_start, RX_DigiPrint.Resources.Language.Resources.Calib));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_step1, RX_DigiPrint.Resources.Language.Resources.Calib1));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_step2, RX_DigiPrint.Resources.Language.Resources.Calib2));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_step3, RX_DigiPrint.Resources.Language.Resources.Calib3));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_step4, RX_DigiPrint.Resources.Language.Resources.Calib4));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_cal_done, RX_DigiPrint.Resources.Language.Resources.CalibDone));

            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_test_watchdog, RX_DigiPrint.Resources.Language.Resources.TestWDG));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_test, RX_DigiPrint.Resources.Language.Resources.Test));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_offset_cal, RX_DigiPrint.Resources.Language.Resources.CalOffset));
            _List.Add(new RxEnum<EFluidCtrlMode>(EFluidCtrlMode.ctrl_offset_cal_done, RX_DigiPrint.Resources.Language.Resources.CalDone));
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<EFluidCtrlMode>>(_List);
        }

    }
}
