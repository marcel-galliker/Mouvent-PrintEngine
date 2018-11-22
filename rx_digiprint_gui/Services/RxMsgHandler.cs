using RX_Common;
using RX_DigiPrint.Models;
using rx_rip_gui.Models;
using rx_rip_gui.Services;
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
            RxGlobals.PrintEnv.Reset();
            RxGlobals.InkTypes.Reset();
        }

        //--- Message Dispatcher ----------------------------------------------------------
        public void handle_message(Byte[] msg)
        {
            TcpIp.SMsgHdr hdr;
            RxStructConvert.ToStruct(out hdr, msg);

            try
            {
                switch (hdr.msgId)
                {
                    case TcpIp.REP_EVT_CONFIRM:     RxGlobals.Events.Reset();   break;
                    case TcpIp.EVT_GET_EVT:         handle_event(msg);          break;
                    case TcpIp.REP_PING:                                        break;
                    case TcpIp.REP_NETWORK_RELOAD:  RxGlobals.Network.Reset();  break;
                    case TcpIp.EVT_NETWORK_ITEM:    handle_net_item(msg);       break;
                    case TcpIp.REP_NETWORK_SETTINGS:handle_net_settings(msg);   break;

                    case TcpIp.REP_GET_PRINT_QUEUE: RxGlobals.PrintQueue.RemoveItem(null);    break;
                    case TcpIp.EVT_GET_PRINT_QUEUE: handle_print_queue(msg);    break;
                    case TcpIp.EVT_DEL_PRINT_QUEUE: handle_print_queue(msg);    break;
                    case TcpIp.EVT_UP_PRINT_QUEUE:  handle_print_queue(msg);    break;
                    case TcpIp.EVT_DN_PRINT_QUEUE:  handle_print_queue(msg);    break;

                    case TcpIp.REP_GET_PRINT_ENV:   handle_print_env(msg);      break;
                    case TcpIp.BEG_GET_PRINT_ENV:   handle_print_env(msg);      break;
                    case TcpIp.ITM_GET_PRINT_ENV:   handle_print_env(msg);      break;
                    case TcpIp.END_GET_PRINT_ENV:   handle_print_env(msg);      break;

                    case TcpIp.REP_GET_PRINTER_CFG: handle_printer_cfg(msg);    break;
                    case TcpIp.REP_SET_PRINTER_CFG:                             break;

                    case TcpIp.REP_GET_STEPPER_CFG: handle_stepper_cfg(msg);    break;

                    case TcpIp.REP_PLC_GET_INFO:    handle_plc_info(msg);        break;       
                    case TcpIp.REP_PLC_GET_LOG:     handle_plc_log(msg);         break;
//                  case TcpIp.END_PLC_GET_LOG:     handle_plc_log_end(msg);     break; 
                                                       
                    case TcpIp.REP_FLUID_STAT:      handle_fluid_stat(msg);      break;
                    case TcpIp.REP_SCALE_STAT:      handle_scale_stat(msg);      break;

                    case TcpIp.REP_CHILLER_STAT:    handle_chiller_stat(msg);    break;

                    case TcpIp.REP_HEAD_STAT:       handle_head_stat(msg);       break;
                    case TcpIp.REP_TT_STAT:         handle_tt_stat(msg);         break;    

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

                    default:
                        RxGlobals.Events.AddItem(new LogItem("Received unknown MessageId=0x{0:X}", hdr.msgId)); 
                        break;
                }
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
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length")); 
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
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length")); 
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
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length")); 
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
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length")); 
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
                    case TcpIp.REP_GET_PRINT_QUEUE: RxGlobals.PrintQueue.RemoveItem(null);    break;
                    case TcpIp.EVT_GET_PRINT_QUEUE: RxGlobals.PrintQueue.AddItem(item);       break;
                    case TcpIp.EVT_DEL_PRINT_QUEUE: RxGlobals.PrintQueue.RemoveItem(item);    break;
                    case TcpIp.EVT_UP_PRINT_QUEUE:  RxGlobals.PrintQueue.UpItem(item);        break;
                    case TcpIp.EVT_DN_PRINT_QUEUE:  RxGlobals.PrintQueue.DownItem(item);      break;
                }
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length"));
        }

        //--- handle_print_env -----------------------------------------
        private void handle_print_env(Byte[] buf)
        {
            TcpIp.sPrintEnvMsg msg;
            int len=RxStructConvert.ToStruct(out msg, buf);
            if (len==msg.hdr.msgLen) 
            {
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
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length")); 
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
                                            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length")); 
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
                RxGlobals.InkSupply.Update(msg);
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length")); 
        }

        //--- handle_scale_stat -----------------------------------------
        private void handle_scale_stat(Byte[] buf)
        {
            int i;
            int offset=8;
            
            for (i=0; i<TcpIp.ScalesCnt; i++)
            {
                RxStructConvert.ToStruct(out RxGlobals.Scales[i], buf, offset);
                offset += Marshal.SizeOf(RxGlobals.Scales[i]);
            }
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
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length")); 
        }

        //--- handle_head_stat -----------------------------------------
        private void handle_head_stat(Byte[] buf)
        {
            TcpIp.SHeadBoardStatMsg msg;

            int len=RxStructConvert.ToStruct(out msg, buf);
            if (len==msg.hdr.msgLen) 
            {
                RxGlobals.HeadStat.SetConnected((int)msg.stat.boardNo, (msg.stat.info&1)!=0);

                int i, no;
                for (i=0; i<TcpIp.HEAD_CNT; i++)
                {
                    no = (int)(msg.stat.boardNo*TcpIp.HEAD_CNT)+i;
                    RxGlobals.HeadStat.SetItem(no, msg.stat.head[i], msg.stat.tempFpga, msg.stat.flow);
                    RxGlobals.HeadStat.List[no].SendBt();
                }
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length"));
        }

        //--- handle_tt_stat -----------------------------------------
        private void handle_tt_stat(Byte[] buf)
        {            
            TcpIp.STestTableStat msg;
            int len=RxStructConvert.ToStruct(out msg, buf);
            if (len==msg.hdr.msgLen)
            {
                RxGlobals.TestTableStatus.Update(msg);
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length")); 
        }

        //--- handle_printer_cfg -----------------------------------------
        private void handle_printer_cfg(Byte[] buf)
        {
            TcpIp.SPrinterCfgMsg msg;
            int len=RxStructConvert.ToStruct(out msg, buf);
            if (len== msg.hdr.msgLen)
            {
                switch(msg.hdr.msgId)
                { 
                    case TcpIp.REP_GET_PRINTER_CFG: RxGlobals.PrintSystem.SetPrintCfg(msg); break;
                }
            }
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length")); 
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
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length")); 
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
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length")); 
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
            else RxGlobals.Events.AddItem(new LogItem("Received invalid message Length")); 
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
            else  RxGlobals.Events.AddItem(new LogItem("Received invalid message Length"));
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
    }
}
