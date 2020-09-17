using RX_Common;
using RX_DigiPrint.Models;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;

namespace RX_DigiPrint.Services
{
    public class RxBtMsgHandler
    {
        //--- constructor ---------------
        public RxBtMsgHandler()
        {
        }

        //--- handle_message ------------------
        public void handle_message(Byte[] msg, RxBtClient client)
        {
            RxBtDef.SMsgHdr hdr;
            RxStructConvert.ToStruct(out hdr, msg);

            try
            {
                switch (hdr.msgId)
                {
                    case RxBtDef.BT_CMD_LOGIN:          handle_login(msg, client); break;
                    case RxBtDef.BT_CMD_BUTTON_1:       handle_button_1(msg, client); break;
                    case RxBtDef.BT_CMD_BUTTON_2:       handle_button_2(msg, client); break;
                    case RxBtDef.BT_CMD_BUTTON_3:       handle_button_3(msg, client); break;
                    case RxBtDef.BT_CMD_BUTTON_4:       handle_button_4(msg, client); break;

                    case RxBtDef.BT_CMD_STATE:          handle_bt_state_req(client); break;

                    case RxBtDef.BT_CMD_JOG_FWD_START:  RxGlobals.Events.AddItem(new LogItem("RxBtMsgHandler: JOG_FWD: START" ));
                                                        RxGlobals.Plc.SetVar("CMD_JOG_FWD", 1); 
                                                        break;

                    case RxBtDef.BT_CMD_JOG_FWD_STOP:   RxGlobals.Events.AddItem(new LogItem("RxBtMsgHandler: JOG_FWD: STOP" ));
                                                        RxGlobals.Plc.SetVar("CMD_JOG_FWD", 0); 
                                                        break;

                    case RxBtDef.BT_CMD_JOG_BWD_START:  RxGlobals.Events.AddItem(new LogItem("RxBtMsgHandler: JOG_BWD: START" ));
                                                        RxGlobals.Plc.SetVar("CMD_JOG_BWD", 1); 
                                                        break;

                    case RxBtDef.BT_CMD_JOG_BWD_STOP:   RxGlobals.Events.AddItem(new LogItem("RxBtMsgHandler: JOG_BWD: STOP" ));   
                                                        RxGlobals.Plc.SetVar("CMD_JOG_BWD", 0); 
                                                        break;

                    case RxBtDef.BT_CMD_EVT_CONFIRM:    RxGlobals.RxInterface.SendCommand(TcpIp.CMD_EVT_CONFIRM); break;

                    case RxBtDef.BT_CMD_PROD_STATE:     handle_prod_state(client); break;
                    case RxBtDef.BT_CMD_PROD_PREVIEW:   handle_prod_preview(client); break;

                    case RxBtDef.BT_CMD_STEPPER_STATE:  handle_stepper_state(client); break;
                    case RxBtDef.BT_CMD_STEPPER_TEST:   handle_stepper_test(msg);   break;
                    case RxBtDef.BT_CMD_STEPPER_STOP:   handle_stepper_cmd(hdr.msgId);    break;
                    case RxBtDef.BT_CMD_STEPPER_REF:    handle_stepper_cmd(hdr.msgId);    break;
                    case RxBtDef.BT_CMD_STEPPER_UP:     handle_stepper_cmd(hdr.msgId);    break;
                    case RxBtDef.BT_CMD_STEPPER_PRINT:  handle_stepper_cmd(hdr.msgId);    break;
                    case RxBtDef.BT_CMD_STEPPER_CAP:    handle_stepper_cmd(hdr.msgId);    break;

                    case RxBtDef.CMD_START_PRINTING:    handle_start_printing(msg, client); break;
                    case RxBtDef.CMD_PAUSE_PRINTING:    RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PAUSE_PRINTING); break;
                    case RxBtDef.CMD_STOP_PRINTING:     RxGlobals.RxInterface.SendCommand(TcpIp.CMD_STOP_PRINTING);  break;
                    case RxBtDef.CMD_ABORT_PRINTING:    RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ABORT_PRINTING); break;

                    case RxBtDef.CMD_FLUID_CTRL_MODE:   handle_fluid_ctrlMode(msg, client); break;


                    default:
                        RxGlobals.Events.AddItem(new LogItem("RxMsgHandler: Received unknown MessageId=0x{0:X}", hdr.msgId)); 
                        break;
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("RxMsgHandler.handle_message (id={0:X}): >>{1}<<", hdr.msgId, e.Message );
                Console.WriteLine("Stack Trace:\n{0}\n", e.StackTrace);
                RxGlobals.Events.AddItem(new LogItem("RxBtMsgHandler.handle_message (id={0:X}): >>%s<<", hdr.msgId, e.Message));
            }
        }

        //--- handle_login -----------------------------------------------------------
        private void handle_login(Byte[] buf, RxBtClient client)
        {
            RxBtDef.SLogInMsg msg;
            RxStructConvert.ToStruct(out msg, buf);
            client.LogIn(msg);            
        }

        //--- handle_button_1 -----------------------------------------------------------
        private void handle_button_1(Byte[] buf, RxBtClient client)
        {
            RxGlobals.Events.AddItem(new LogItem("RxBtMsgHandler: Button 1" ));
        }

        //--- handle_button_2 -----------------------------------------------------------
        private void handle_button_2(Byte[] buf, RxBtClient client)
        {
            RxGlobals.Events.AddItem(new LogItem("RxBtMsgHandler: Button 2" ));
        }

        //--- handle_button_3 -----------------------------------------------------------
        private void handle_button_3(Byte[] buf, RxBtClient client)
        {
            RxBtDef.SReplyMsg msg;
            RxStructConvert.ToStruct(out msg, buf);
            if (msg.reply==0) RxGlobals.Events.AddItem(new LogItem("RxBtMsgHandler: Button 3-UP" ));
            if (msg.reply==1) RxGlobals.Events.AddItem(new LogItem("RxBtMsgHandler: Button 3-DOWN" ));
        }

        //--- handle_button_4 -----------------------------------------------------------
        private void handle_button_4(Byte[] buf, RxBtClient client)
        {
            RxGlobals.Events.AddItem(new LogItem("RxBtMsgHandler: Button 4" ));
        }

        //--- handle_fluid_ctrlMode ---------------------------------------------
        private void handle_fluid_ctrlMode(Byte[] buf, RxBtClient client)
        {
            RxBtDef.SBtFluidCtrlCmd btmsg;
            RxStructConvert.ToStruct(out btmsg, buf);
            TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
            msg.no       = btmsg.no;
            msg.ctrlMode = (EFluidCtrlMode)btmsg.ctrlMode;

            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_HEAD_FLUID_CTRL_MODE, ref msg);
        }

        //--- handle_start_printing -------------------------------------
        private void handle_start_printing(Byte[] buf, RxBtClient client)
        {
            RxBtDef.SReplyMsg msg;
            RxStructConvert.ToStruct(out msg, buf);
            if (msg.reply!=0)
            {
                TcpIp.SFluidCtrlCmd cmd = new TcpIp.SFluidCtrlCmd(){no=-1, ctrlMode = EFluidCtrlMode.ctrl_print};
                RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref cmd);
            }
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_START_PRINTING);
        }

        //--- handle_bt_state_req -----------------------------------------------------------
        private RxBtDef.EBTState _printerState;

        public void handle_bt_state_req(RxBtClient client)
        {
            RxBtDef.SReplyStateMsg msg = new RxBtDef.SReplyStateMsg();
            switch(RxGlobals.PrinterStatus.PrintState)
            {
                case EPrintState.ps_webin:      msg.state=RxBtDef.EBTState.state_webin;     break;
                case EPrintState.ps_goto_pause:
                case EPrintState.ps_printing:
                case EPrintState.ps_stopping:   msg.state=RxBtDef.EBTState.state_prouction; break;
                case EPrintState.ps_off:
                case EPrintState.ps_ready_power:msg.state=RxBtDef.EBTState.state_stop;      break;
                default:                        msg.state=RxBtDef.EBTState.state_undef;     break;
            }
            _printerState       = msg.state;
            msg.colorCnt        = RxGlobals.PrintSystem.ColorCnt;
            msg.headsPerColor   = RxGlobals.PrintSystem.HeadsPerColor;
            if (RxGlobals.PrintSystem.IsTx)
                 msg.reverse = 1;
            else msg.reverse = 0;

       //     RxGlobals.Events.AddItem(new LogItem("TEST Set Bluetioth.State to WEBIN"));
       //     msg.state=RxBtDef.EBTState.state_webin;
            client.SendMsg(RxBtDef.BT_REP_STATE,  ref msg);
            handle_prod_state(client);
        }

        //--- handle_prod_state -----------------------------------------------------
        private PrintQueueItem _LastItem = new PrintQueueItem();
        public void handle_prod_state(RxBtClient client)
        {
            if (_printerState==RxBtDef.EBTState.state_prouction) 
            {
                if (RxGlobals.PrintQueue.Printed.Count>0) _LastItem=RxGlobals.PrintQueue.Printed[0];
                else _LastItem.State = EPQState.queued;
            }
            else 
            {
                if (RxGlobals.PrintQueue.Queue.Count>0) _LastItem=RxGlobals.PrintQueue.Queue[0];
                else _LastItem.State = EPQState.undef;
            }
            _LastItem.SendBtProdState();
        }

        //--- handle_prod_preview -----------------------------------------------------
        public void handle_prod_preview(RxBtClient client)
        {
            try
            {
                PrintQueueItem item=null;
                if (RxGlobals.PrinterStatus.PrintState==EPrintState.ps_printing || RxGlobals.PrinterStatus.PrintState==EPrintState.ps_stopping)
                {
                    if (RxGlobals.PrintQueue.Printed.Count>0) item=RxGlobals.PrintQueue.Printed[0];
                }
                else
                {
                    if (RxGlobals.PrintQueue.Queue.Count>0) item=RxGlobals.PrintQueue.Queue[0];
                }
                if (item!=null)
                {
                    FileStream file = File.Open(item.PreviewPath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
                    byte[] data = new byte[file.Length];
                    file.Read(data, 0, (int)file.Length);
                    file.Close();
                    client.SendMsgBuf(RxBtDef.BT_REP_PROD_PREVIEW, data);
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception {0}", e.Message);
            }
        }

        //--- handle_stepper_state -----------------------------------------------------
        public void handle_stepper_state(RxBtClient client)
        {
            foreach (StepperMotor motor in RxGlobals.StepperStatus[0].Motors)
                motor.SendBt(client);
        }

        //--- handle_stepper_test --------------------
        private void handle_stepper_test(Byte[] buf)
        {
            RxBtDef.SBtStepperTestMsg msg;
            RxStructConvert.ToStruct(out msg, buf);
            if (RxGlobals.StepperStatus[0].Motors.Count() > msg.motorNo)
                RxGlobals.StepperStatus[0].Motors[msg.motorNo].DoTest(msg.moveUp != 0);
        }

        //--- handle_stepper_cmd ----------------------
        private void handle_stepper_cmd(UInt32 cmd)
        {
            switch(cmd)
            {
                case RxBtDef.BT_CMD_STEPPER_STOP:   RxGlobals.RxInterface.SendCommand(TcpIp.CMD_LIFT_STOP);          break;
                case RxBtDef.BT_CMD_STEPPER_REF:    RxGlobals.RxInterface.SendCommand(TcpIp.CMD_LIFT_REFERENCE);     break;
                case RxBtDef.BT_CMD_STEPPER_UP:     RxGlobals.RxInterface.SendCommand(TcpIp.CMD_LIFT_UP_POS);        break;
                case RxBtDef.BT_CMD_STEPPER_PRINT:  RxGlobals.RxInterface.SendCommand(TcpIp.CMD_LIFT_PRINT_POS);     break;
                case RxBtDef.BT_CMD_STEPPER_CAP:    RxGlobals.RxInterface.SendCommand(TcpIp.CMD_LIFT_CAPPING_POS);   break;
                default:                                                                                            break;
            }
        }
    }
}
