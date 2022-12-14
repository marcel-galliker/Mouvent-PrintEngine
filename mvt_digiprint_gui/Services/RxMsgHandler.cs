using RX_Common;
using RX_DigiPrint.Models;
using RxRexrothGui.Models;
using RxRexrothGui.Services;
using System;
using System.Runtime.InteropServices;

namespace RX_DigiPrint.Services
{
    public class RxMsgHandler
    {
        //--- Restart --------------------------------------------------------------------
        public  void Restart()
        {
            RxGlobals.Log.Reset();
            RxGlobals.Events.Reset();
            RxGlobals.Network.Reset();
            RxGlobals.PrintQueue.Reset();
            RxGlobals.InkTypes.Reset();
        }

        //--- Message Dispatcher ----------------------------------------------------------
        public TcpIp.SMsgHdr hdr;
        public void handle_message(Byte[] msg)
        {
//            TcpIp.SMsgHdr hdr;
            RxStructConvert.ToStruct(out hdr, msg);

            try
            {
                // Console.WriteLine("{0}: handle_message id=0x{1:X}", RxGlobals.Timer.Ticks(), hdr.msgId);
                //   RxGlobals.Events.AddItem(new LogItem(string.Format("handle_message id={0:X}, len={1}", hdr.msgId, hdr.msgLen)));
                switch (hdr.msgId)
                {
                    case TcpIp.REP_EVT_CONFIRM:     RxGlobals.Events.Reset();   break;
                    case TcpIp.EVT_GET_EVT:         handle_event(msg);          break;
                    case TcpIp.REP_PING:                                        break;
                    case TcpIp.REP_NETWORK_RELOAD:  RxGlobals.Network.Reset();  break;
                    case TcpIp.EVT_NETWORK_ITEM:    handle_net_item(msg);       break;
                    case TcpIp.REP_NETWORK_SETTINGS:handle_net_settings(msg);   break;

                    case TcpIp.REP_GET_PRINT_QUEUE: RxGlobals.PrintQueue.RemoveItem(null);    break;
                    case TcpIp.EVT_ADD_PRINT_QUEUE: handle_print_queue(msg);    break;
                    case TcpIp.EVT_GET_PRINT_QUEUE: handle_print_queue(msg);    break;
                    case TcpIp.EVT_DEL_PRINT_QUEUE: handle_print_queue(msg);    break;
                    case TcpIp.EVT_UP_PRINT_QUEUE:  handle_print_queue(msg);    break;
                    case TcpIp.EVT_DN_PRINT_QUEUE:  handle_print_queue(msg);    break;
                    case TcpIp.REP_CHG_PRINT_QUEUE: handle_change_job();        break;

                    case TcpIp.REP_GET_PRINT_ENV:   handle_print_env(msg);      break;
                    case TcpIp.BEG_GET_PRINT_ENV:   handle_print_env(msg);      break;
                    case TcpIp.ITM_GET_PRINT_ENV:   handle_print_env(msg);      break;
                    case TcpIp.END_GET_PRINT_ENV:   handle_print_env(msg);      break;

                    case TcpIp.REP_GET_PRINTER_CFG: handle_printer_cfg(msg);    break;
                    case TcpIp.REP_SET_PRINTER_CFG:                             break;

                    case TcpIp.REP_GET_STEPPER_CFG: handle_stepper_cfg(msg);    break;
                    case TcpIp.REP_STEPPER_STAT: handle_stepper_stat(msg);      break;

                    case TcpIp.REP_PLC_GET_INFO:    handle_plc_info(msg);        break;       
                    case TcpIp.REP_PLC_GET_LOG:     handle_plc_log(msg);         break;
//                  case TcpIp.END_PLC_GET_LOG:     handle_plc_log_end(msg);     break; 
                                                       
                    case TcpIp.REP_FLUID_STAT:      handle_fluid_stat(msg);      break;

                    case TcpIp.REP_CHILLER_STAT:    handle_chiller_stat(msg);    break;
                    case TcpIp.REP_ENCODER_STAT:    handle_encoder_stat(msg);    break;
                    case TcpIp.REP_HEAD_STAT:       handle_head_stat(msg);       break;
                    case TcpIp.REP_SETUP_ASSIST_STAT:handle_setup_assist_stat(msg); break;
                    
                    case TcpIp.REP_PLC_GET_VAR:     handle_plc_var(msg);         break;
                    case TcpIp.REP_PLC_LOAD_PAR:    handle_plc_load_par(msg);    break;

                    case TcpIp.CMD_PLC_RES_MATERIAL:    handle_plc_res_material(RxGlobals.MaterialList);  break;
                    case TcpIp.CMD_PLC_ITM_MATERIAL:    handle_plc_itm_material(RxGlobals.MaterialList, msg);break;
                    case TcpIp.CMD_PLC_DEL_MATERIAL:    handle_plc_del_material(RxGlobals.MaterialList, msg);break;

                    case TcpIp.REP_GET_INK_DEF:     handle_ink_def(msg);        break;
                    case TcpIp.BEG_GET_INK_DEF:     handle_ink_def(msg);        break;
                    case TcpIp.ITM_GET_INK_DEF:     handle_ink_def(msg);        break;
                    case TcpIp.END_GET_INK_DEF:     handle_ink_def(msg);        break;

                    case TcpIp.EVT_PRINTER_STAT:    handle_printer_stat(msg);   break;

                    case TcpIp.REP_REQ_LOG:         handle_event_req(msg);      break;
                    case TcpIp.EVT_GET_LOG:         handle_event(msg);          break;

                    case TcpIp.REP_GET_DENSITY:       handle_density(msg); break;

                    case TcpIp.REP_CO_GET_ORDER:       handle_co_order(msg);                break;
                    case TcpIp.REP_CO_SET_OPERATOR:    handle_co_operator(msg);           break;
                    case TcpIp.REP_CO_GET_PRODUCTION:  handle_co_production(msg);           break;
                    case TcpIp.REP_CO_GET_ROLLS:       RxGlobals.CleafOrder.Rolls.Clear();  break;
                    case TcpIp.REP_CO_SET_ROLL:        handle_co_roll(msg);                 break;
                                
                    default:
                        RxGlobals.Events.AddItem(new LogItem("Received unknown MessageId=0x{0:X}", hdr.msgId)); 
                        break;
                }
            //    Console.WriteLine("{0}: handle_message id=0x{1:X} DONE", RxGlobals.Timer.Ticks(), hdr.msgId);
            }
            catch (Exception e)
            {
                Console.WriteLine("RxMsgHandler.handle_message (id={0:X}): >>{1}<<", hdr.msgId, e.Message );
                Console.WriteLine("Stack Trace:\n{0}\n", e.StackTrace);
                if (RxGlobals.RxInterface.Running) RxGlobals.Log.AddItem(new LogItem("RxMsgHandler.handle_message (id={0:X}): >>%s<<", hdr.msgId, e.Message));
            }
        }

        //--- Message Handlers -----------------------------------------------------------
        private void handle_event_req(Byte[] buf)
        {
            TcpIp.SLogReqMsg req;
            int len=RxStructConvert.ToStruct(out req, buf);
            if (len==req.hdr.msgLen) 
            {
                RxGlobals.Log.ItemCount = req.count;
                RxGlobals.Log.Pos       = req.first;
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SLogReqMsg")); 
        }

        //--- handle_event ----------------------------------------------------------------
        private void handle_event(Byte[] buf)
        {
            TcpIp.SErrorMsg err;
            int len=RxStructConvert.ToStruct(out err, buf);
            if (len==err.hdr.msgLen)
            {
                LogItem item = new LogItem(err);
                switch(err.hdr.msgId)
                {
                case TcpIp.EVT_GET_EVT: RxGlobals.Events.AddItem(item);
                                        item.SendToBluetooth();
                                        break;
                case TcpIp.EVT_GET_LOG: RxGlobals.Log.   AddItem(item); break;
                default: RxGlobals.Events.AddItem(new LogItem("Received unknown MessageId=0x{0:X}", err.hdr.msgId)); break;
                }
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SErrorMsg")); 
        }

        //--- handle_net_item --------------------------------------------
        private void handle_net_item(Byte[] buf)
        {
            TcpIp.sNetworkMsg msg;
            int len= RxStructConvert.ToStruct(out msg, buf);
            if (len==msg.hdr.msgLen)
            {
                RxGlobals.Network.AddItem(new NetworkItem(ref msg.item, msg.flash));
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length sNetworkMsg")); 
        }

        //--- handle_net_settings -----------------------------------------
        private void handle_net_settings(Byte[] buf)
        {
            TcpIp.SIfConfig settings;
            int len=RxStructConvert.ToStruct(out settings, buf);
            if (len==settings.hdr.msgLen)
            { 
                RxBindable.Invoke(()=>RxGlobals.PrinterProperties.NetworkSettings(settings));
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SIfConfig")); 
        }

        //--- handle_print_queue -----------------------------------------
        private void handle_print_queue(Byte[] buf)
        {
            TcpIp.sPrintQueueMsg msg;
            int len = RxStructConvert.ToStruct(out msg, buf);
            if (len==msg.hdr.msgLen) 
            {
                PrintQueueItem item = new PrintQueueItem(msg.item);

                switch(msg.hdr.msgId)
                { 
                    case TcpIp.REP_GET_PRINT_QUEUE: RxGlobals.PrintQueue.RemoveItem(null);     break;
                    case TcpIp.EVT_GET_PRINT_QUEUE: RxGlobals.PrintQueue.AddItem(item, false); break;
                    case TcpIp.EVT_ADD_PRINT_QUEUE: RxGlobals.PrintQueue.AddItem(item, true);  break;
                    case TcpIp.EVT_DEL_PRINT_QUEUE: RxGlobals.PrintQueue.RemoveItem(item);     break;
                    case TcpIp.EVT_UP_PRINT_QUEUE:  RxGlobals.PrintQueue.UpItem(item);         break;
                    case TcpIp.EVT_DN_PRINT_QUEUE:  RxGlobals.PrintQueue.DownItem(item);       break;
                }
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length sPrintQueueMsg"));
        }

        //--- handle_change_job -----------------------------------------
        private void handle_change_job()
		{
            if (RxGlobals.LH702_Preview!=null) RxGlobals.LH702_Preview.ChangeJob(true);
		}

        //--- handle_print_env -----------------------------------------
        private void handle_print_env(Byte[] buf)
        {
            TcpIp.sPrintEnvMsg msg;
            int len=RxStructConvert.ToStruct(out msg, buf);
            if (len==msg.hdr.msgLen) 
            {
                /*
                switch(msg.hdr.msgId)
                { 
                    case TcpIp.BEG_GET_PRINT_ENV: RxGlobals.PrintEnv.Reset();                break;
                    case TcpIp.ITM_GET_PRINT_ENV: 
                        {
                            SPrintEnv cfg=new SPrintEnv()
                                {   name=msg.printEnv,
                                    screening = new SScreeningCfg() {},
                                    ripping   = new SRippingCfg()   {},
                                };
                            RxGlobals.PrintEnv.AddItem(new PrEnv(){Cfg=cfg});  
                        }
                        break;
                }
                 * */
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length sPrintEnvMsg")); 
        }

        //--- handle_ink_def -----------------------------------------
        private void handle_ink_def(Byte[] buf)
        {
            TcpIp.SInkDefMsg msg;
            int len=RxStructConvert.ToStruct(out msg, buf);
            
            switch(msg.hdr.msgId)
            { 
                case TcpIp.BEG_GET_INK_DEF: RxGlobals.InkTypes.Reset();                          break;
                case TcpIp.ITM_GET_INK_DEF: if (len== msg.hdr.msgLen) RxGlobals.InkTypes.AddItem(new InkType(msg.ink));    
                                            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SInkDefMsg")); 
                                            break;
            }
        }

        //--- handle_fluid_stat -----------------------------------------
        private void handle_fluid_stat(Byte[] buf)
        {
            TcpIp.SInkSupplyStatMsg msg;
            int len=RxStructConvert.ToStruct(out msg, buf);
            if (len==msg.hdr.msgLen)
            {
//              Console.WriteLine("handle_fluid_stat[{0}]", msg.no);
                RxGlobals.InkSupply.Update(msg.no, msg.status);
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SInkSupplyStatMsg")); 
        }

        //--- handle_chiller_stat -----------------------------------------
        private void handle_chiller_stat(Byte[] buf)
        {
            TcpIp.SChillerStatMsg msg;
            int len=RxStructConvert.ToStruct(out msg, buf);
            if (len==msg.hdr.msgLen) 
            {
                RxGlobals.Chiller.Update(msg.status);
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SChillerStatMsg")); 
        }

        //--- handle_encoder_stat -----------------------------------------
        private void handle_encoder_stat(Byte[] buf)
        {
            TcpIp.SEncoderStat status;
            TcpIp.SMsgHdr hdr;
            int i;
            int hdrlen = RxStructConvert.ToStruct(out hdr, buf);
            for(i=0; i<2; i++)
            {
                int len=RxStructConvert.ToStruct(out status, buf, hdrlen);
                RxGlobals.Encoder[i].Update(status);
                hdrlen += len;
            }
            if (hdrlen!=hdr.msgLen)
                RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SEncoderStat"));
        }        
        
        //--- handle_head_stat -----------------------------------------
        private void handle_head_stat(Byte[] buf)
        {
            TcpIp.SHeadBoardStatMsg msg;

            int len=RxStructConvert.ToStruct(out msg, buf);

            if (len==msg.hdr.msgLen) 
            {
                RxGlobals.HeadStat.SetConnected((int)msg.stat.boardNo, (msg.stat.info&1)!=0);
                RxGlobals.ClusterStat.SetItem(msg.stat);

                RxGlobals.HeadStat.Meniscus_disabled((int)msg.stat.boardNo, (msg.stat.info & 4) != 0);
                RxGlobals.HeadStat.Cooler_Pressure((int)msg.stat.boardNo, msg.stat.flow);
                RxGlobals.HeadStat.Cooler_Temp((int)msg.stat.boardNo, msg.stat.cooler_temp);
                RxGlobals.HeadStat.FP_Voltage((int)msg.stat.boardNo, msg.stat.fp_voltage);
                RxGlobals.HeadStat.ClusterNo((int)msg.stat.boardNo, msg.stat.clusterNo);
                RxGlobals.HeadStat.SetClusterErr((int)msg.stat.boardNo, msg.stat.err);      

                int i, no;
                for (i=0; i<TcpIp.HEAD_CNT; i++)
                {
                    no = (int)(msg.stat.boardNo*TcpIp.HEAD_CNT)+i;
                    RxGlobals.HeadStat.SetItem(no, msg.stat.head[i], msg.stat.tempFpga, msg.stat.flow);
                    RxGlobals.HeadStat.List[no].SendBt();
                }
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SHeadBoardStatMsg"));
        }

        //--- handle_density -----------------------------------------
        private void handle_density(Byte[] buf)
        {
            TcpIp.SDensityMsg msg;
            int len = RxStructConvert.ToStruct(out msg, buf);
            if (len == hdr.msgLen)
            {
                if (RxGlobals.Density != null) RxBindable.Invoke(() => RxGlobals.Density.SetDensityValues(msg.data.voltage, msg.data.densityValue));
                if (RxGlobals.DisabledJets != null) RxBindable.Invoke(() => RxGlobals.DisabledJets.SetDisablesJets(msg.data.disabledJets));
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SDensityMsg"));
        }

        //--- handle_stepper_stat -----------------------------------------
        private void handle_stepper_stat(Byte[] buf)
        {
            TcpIp.SStepperStat msg;
            int len = RxStructConvert.ToStruct(out msg, buf);
            if (len == msg.hdr.msgLen)
            {
                RxGlobals.StepperStatus[msg.no].Update(msg);
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SStepperStat"));
        }        

        //--- handle_setup_assist_stat -------------------------
        
        private void handle_setup_assist_stat(Byte[] buf)
        {
            TcpIp.SStepperStat msg;

            int len=RxStructConvert.ToStruct(out msg, buf);

            if (len==msg.hdr.msgLen) 
            {
                RxGlobals.SetupAssist.Update(msg);
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SSetupAssistStatMsg"));
        }

        //--- handle_printer_cfg -----------------------------------------
        private void handle_printer_cfg(Byte[] buf)
        {
            TcpIp.SPrinterCfgMsg msg;
            int len = RxStructConvert.ToStruct(out msg, buf);
            if (len == msg.hdr.msgLen)
            {
                RxGlobals.PrintSystem.SetPrintCfg(msg);
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SPrinterCfgMsg"));
        }
        
        //--- handle_printer_stat -----------------------------------------
        private void handle_printer_stat(Byte[] buf)
        {
            TcpIp.SPrinterStatusMsg msg;
            int len=RxStructConvert.ToStruct(out msg, buf);
            if (len==msg.hdr.msgLen)
            {
                RxGlobals.PrinterStatus.Upadte(msg);
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SPrinterStatusMsg")); 
        }

        //--- handle_stepper_cfg -----------------------------------------
        private void handle_stepper_cfg(Byte[] buf)
        {
            TcpIp.SStepperCfgMsg msg;
            int len=RxStructConvert.ToStruct(out msg, buf);
            if (len==msg.hdr.msgLen)
            {
                RxGlobals.Stepper.SetStepperCfg(msg);
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SStepperCfgMsg")); 
        }

        //--- handle_plc_info -----------------------------------------
        private void handle_plc_info(Byte[] buf)
        {
            RxRexroth.SSystemInfo info;
            TcpIp.SMsgHdr hdr;
            int hdrlen = RxStructConvert.ToStruct(out hdr, buf);
            int len=RxStructConvert.ToStruct(out info, buf, hdrlen);
            if (hdrlen+len == hdr.msgLen)
            {
                RxGlobals.Plc.Info = info;
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SSystemInfo")); 
        }

        //--- handle_plc_log -----------------------------------------
        private void handle_plc_log(Byte[] buf)
        {
            RxRexroth.SPlcLogItemMsg msg;
            int len=RxStructConvert.ToStruct(out msg, buf);
            if (len==msg.hdr.msgLen)
            {
                if (msg.item.no%10==0) Console.WriteLine("{0}: Get Log.no={1}", Environment.TickCount, msg.item.no);
                RxBindable.Invoke(()=>RxGlobals.Plc.AddLogItem(new CPlcLogItem(msg.item)));
            }
            else  RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SPlcLogItemMsg"));
        }

        /*
        //--- handle_plc_log_end -----------------------------------------
        private void handle_plc_log_end(Byte[] buf)
        {
            Console.WriteLine("{0}: Get Log end", Environment.TickCount);
            if (RxGlobals.Plc.LogComplete!=null ) RxBindable.Invoke(RxGlobals.Plc.LogComplete);
        }
        */

        //--- handle_plc_var -----------------------------------------
        private void handle_plc_var(Byte[] buf)
        {
            TcpIp.SMsgHdr hdr;
            int len=RxStructConvert.ToStruct(out hdr, buf);
            RxGlobals.Plc.UpdateVar(System.Text.Encoding.UTF8.GetString(buf).Substring(8, hdr.msgLen-8));
        }

        //--- handle_plc_load_par -----------------------------------------
        private void handle_plc_load_par(Byte[] buf)
        {
            RxGlobals.Plc.RequestAllPar();            
        }

        //--- handle_plc_res_material ------------------------------------
        private void handle_plc_res_material(MaterialList list)
        {
            list.Reset();
        }

        //--- handle_plc_itm_material -------------------------------------
        private void handle_plc_itm_material(MaterialList list, Byte[] buf)
        {
            TcpIp.SMsgHdr hdr;
            int len=RxStructConvert.ToStruct(out hdr, buf);
            list.AddItem(new Material(System.Text.Encoding.UTF8.GetString(buf).Substring(8, hdr.msgLen-8)));
        }

        //--- handle_plc_del_material -------------------------------------
        private void handle_plc_del_material(MaterialList list, Byte[] buf)
        {
            TcpIp.SMsgHdr hdr;
            int len=RxStructConvert.ToStruct(out hdr, buf);
            list.DeleteByName(System.Text.Encoding.UTF8.GetString(buf).Substring(8, hdr.msgLen-8));
        }

        //--- handle_co_order -------------------------------------
        private void handle_co_order(Byte[] buf)
        {
            TcpIp.SCleafOrder order;
            TcpIp.SMsgHdr hdr;
            int hdrlen = RxStructConvert.ToStruct(out hdr, buf);
            int len=RxStructConvert.ToStruct(out order, buf, hdrlen);
            if (hdrlen+len == hdr.msgLen)
            {
                RxBindable.Invoke(()=>RxGlobals.CleafOrder.Update_Order(order));
            }
            else  RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SCleafOrder"));
        }

        //--- handle_co_production -------------------------------------
        private void handle_co_production(Byte[] buf)
        {
            TcpIp.SCleafProduction prod;
            TcpIp.SMsgHdr hdr;
            int hdrlen = RxStructConvert.ToStruct(out hdr, buf);
            int len=RxStructConvert.ToStruct(out prod, buf, hdrlen);
            if (hdrlen+len == hdr.msgLen)
            {
                RxBindable.Invoke(()=>RxGlobals.CleafOrder.Update_Prodction(prod));
            }
            else  RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SCleafProduction"));
        }

        //--- handle_co_roll -------------------------------------
        private void handle_co_roll(Byte[] buf)
        {
            TcpIp.SCleafRoll roll;
            TcpIp.SMsgHdr hdr;
            int hdrlen = RxStructConvert.ToStruct(out hdr, buf);
            int len=RxStructConvert.ToStruct(out roll, buf, hdrlen);
            if (hdrlen+len == hdr.msgLen)
            {
                RxBindable.Invoke(()=>RxGlobals.CleafOrder.Update_Roll(roll));
            }
            else  RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SCleafRoll"));
        }

        //--- handle_co_operator -------------------------------------
        private void handle_co_operator(Byte[] buf)
        {
            TcpIp.SCleafRoll roll;
            TcpIp.SMsgHdr hdr;
            int hdrlen = RxStructConvert.ToStruct(out hdr, buf);
            int len=RxStructConvert.ToStruct(out roll, buf, hdrlen);
            if (hdrlen+len == hdr.msgLen)
            {
                RxBindable.Invoke(()=>RxGlobals.CleafOrder.User= roll.user);
            }
            else  RxGlobals.Events.AddItem(new LogItem("Received invalid message Length SCleafRoll"));
        }
    }
}
